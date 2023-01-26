/*!
\file Comm_MPI.cc
*/

#ifdef HAVE_MPI

#include "Ume/Comm_MPI.hh"
#include <cassert>
#include <mpi.h>
#include <vector>

namespace Ume {
namespace Comm {

MPI::MPI(int *argc, char ***argv, int const virtual_rank)
    : virtual_rank_{virtual_rank} {
  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &numpe_);
  std::vector<int> r2v(numpe_, -1);
  r2v[rank_] = virtual_rank_;
  MPI_Allgather(MPI_IN_PLACE, numpe_, MPI_INT, r2v.data(), numpe_, MPI_INT,
      MPI_COMM_WORLD);
  for (int i = 0; i < numpe_; ++i) {
    v2r_rank_.insert(std::make_pair(r2v[i], i));
  }
  int flag;
  MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_TAG_UB, &max_tag_, &flag);
}

int MPI::get_tag() {
  static int curr_tag = 1;
  if (curr_tag >= max_tag_)
    curr_tag = 1;
  return curr_tag++;
}

template <class T> struct MPI_Datatype_Map {};
template <> struct MPI_Datatype_Map<int> {
  static MPI_Datatype mpi_type() { return MPI_INT; }
};

template <> struct MPI_Datatype_Map<double> {
  static MPI_Datatype mpi_type() { return MPI_DOUBLE; }
};

template <class T>
int exchange_impl(MPI &comm_mpi, Buffers<T> const &sends, Buffers<T> &recvs) {
  using base_type = typename Buffers<T>::base_type;
  MPI_Datatype const msgtype = MPI_Datatype_Map<base_type>::mpi_type();
  int const tag = comm_mpi.get_tag();

  size_t const nrecvs = recvs.remotes.size();
  size_t const nsends = sends.remotes.size();
  std::vector<MPI_Request> reqs(nrecvs + nsends);

  /* Post the non-blocking receives */
  for (size_t i = 0; i < nrecvs; ++i) {
    base_type *start = &(recvs.buf[recvs.remotes[i].buf_offset]);
    int const rmtpe = comm_mpi.translate_pe(recvs.remotes[i].pe);
    int stat = MPI_Irecv(start, recvs.remotes[i].buf_len, msgtype, rmtpe, tag,
        MPI_COMM_WORLD, &(reqs[i]));
    assert(stat == MPI_SUCCESS);
  }

  /* Post non-blocking sends */
  for (size_t i = 0; i < nsends; ++i) {
    base_type const *start = &(sends.buf[sends.remotes[i].buf_offset]);
    int const rmtpe = comm_mpi.translate_pe(sends.remotes[i].pe);
    int stat = MPI_Isend(start, sends.remotes[i].buf_len, msgtype, rmtpe, tag,
        MPI_COMM_WORLD, &(reqs[i + nrecvs]));
    assert(stat == MPI_SUCCESS);
  }

  /* Wait for MPI to work through all of that */
  std::vector<MPI_Status> stats(nrecvs + nsends);
  MPI_Waitall(static_cast<int>(nrecvs + nsends), reqs.data(), stats.data());

  return 0;
}

void MPI::exchange(
    Buffers<DS_Types::INTV_T> const &sends, Buffers<DS_Types::INTV_T> &recvs) {
  exchange_impl(*this, sends, recvs);
}
void MPI::exchange(
    Buffers<DS_Types::DBLV_T> const &sends, Buffers<DS_Types::DBLV_T> &recvs) {
  exchange_impl(*this, sends, recvs);
}
void MPI::exchange(Buffers<DS_Types::VEC3V_T> const &sends,
    Buffers<DS_Types::VEC3V_T> &recvs) {
  exchange_impl(*this, sends, recvs);
}

int MPI::stop() {
  MPI_Finalize();
  return 0;
}

} // namespace Comm
} // namespace Ume

#endif
