/*!
  \file ume_serial.cc

  This is an example of a serial driver for Ume.  It simply reads one or more
  binary Ume file(s) and accesses some computed variables in the first one.
*/

/*
** Scoria Includes
*/
#if defined(USE_SCORIA) && defined(USE_CLIENT)
extern "C" {
#include "scoria.h"
}
#endif

/*
** Ume Includes
*/
#include "Ume/SOA_Idx_Mesh.hh"
#include "shm_allocator.hh"
#include <fstream>
#include <iostream>
#include <vector>

using namespace Ume::SOA_Idx;

UmeVector<Mesh> read_meshes(int argc, char *argv[]);

int main(int argc, char *argv[]) {
#if defined(USE_SCORIA) && defined(USE_CLIENT)
  struct client client;
  client.chatty = 0;

  scoria_init(&client);
#endif

  UmeVector<Mesh> ranks{read_meshes(argc, argv)};

  if (ranks.empty())
    return 1;

  Ume::Comm::Dummy_Transport comm;
  ranks[0].comm = &comm;

  auto const &test = ranks[0].ds->caccess_vec3v("corner_csurf");
  auto const &test2 = ranks[0].ds->caccess_vec3v("side_surz");
  auto const &test3 = ranks[0].ds->caccess_vec3v("point_norm");

#if defined(USE_SCORIA) && defined(USE_CLIENT)
  struct request req;
  scoria_quit(&client, &req);
  scoria_wait_request(&client, &req);

  scoria_cleanup(&client);
#endif

  return 0;
}

UmeVector<Mesh> read_meshes(int const argc, char *argv[]) {
  UmeVector<Mesh> ranks;
  ranks.resize(argc - 1);
  bool need_sort{false};
  if (argc == 1) {
    std::cerr << "Usage: ume_serial <ume file>+" << std::endl;
    std::exit(1);
  }
  for (int i = 1; i < argc; ++i) {
    std::cout << "Reading: " << argv[i] << '\n';
    std::ifstream is(argv[i]);
    if (!is) {
      std::cerr << "Unable to open file \"" << argv[i] << "\" for reading."
                << std::endl;
      return UmeVector<Mesh>{};
    }
    ranks[i - 1].read(is);
    if (ranks[i - 1].mype != i - 1)
      need_sort = true;
  }
  size_t const numpe = static_cast<size_t>(ranks[0].numpe);
  if (numpe != ranks.size()) {
    std::cerr << "Warning: the initial mesh had " << numpe
              << " ranks, but only " << ranks.size() << " were read"
              << std::endl;
  }
  if (need_sort) {
    std::cerr << "Warning: sorting input ranks" << std::endl;
    std::sort(ranks.begin(), ranks.end(),
        [](Mesh const &a, Mesh const &b) { return a.mype < b.mype; });
  }

  return ranks;
}
