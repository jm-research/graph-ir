#ifndef GRAPHIR_CODEGEN_BGL_H
#define GRAPHIR_CODEGEN_BGL_H
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/properties.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include "graphir/CodeGen/BasicBlock.h"
#include "graphir/CodeGen/GraphScheduling.h"
#include "graphir/Support/STLExtras.h"
#include "graphir/Support/type_traits.h"
#include "graphir/Support/Graph.h"
#include <utility>

namespace boost {
template<>
struct graph_traits<graphir::GraphSchedule> {
  /// GraphConcept
  using vertex_descriptor = graphir::BasicBlock*;
  // { source, dest }
  using edge_descriptor = std::pair<vertex_descriptor, vertex_descriptor>;
  using directed_category = boost::directed_tag;
  using edge_parallel_category = boost::allow_parallel_edge_tag;

  static vertex_descriptor null_vertex() {
    return nullptr;
  }

  /// VertexListGraphConcept
  using vertex_iterator = typename graphir::GraphSchedule::rpo_iterator;
  using vertices_size_type = size_t;

  /// EdgeListGraphConcept
  using edge_iterator = typename graphir::GraphSchedule::edge_iterator;
  using edges_size_type = size_t;

  /// IncidenceGraphConcept
  using out_edge_iterator
    = boost::transform_iterator<typename graphir::BasicBlock::EdgeBuilder,
                                typename graphir::BasicBlock::succ_iterator,
                                edge_descriptor, // Reference type
                                edge_descriptor // Value type
                                >;
  using degree_size_type = size_t;

  struct traversal_category :
    public boost::vertex_list_graph_tag,
    public boost::edge_list_graph_tag,
    public boost::incidence_graph_tag {};
};

/// VertexListGraphConcept
inline
std::pair<typename boost::graph_traits<graphir::GraphSchedule>::vertex_iterator,
          typename boost::graph_traits<graphir::GraphSchedule>::vertex_iterator>
vertices(graphir::GraphSchedule& g) {
  return std::make_pair(g.rpo_begin(), g.rpo_end());
}
inline
std::pair<typename boost::graph_traits<graphir::GraphSchedule>::vertex_iterator,
          typename boost::graph_traits<graphir::GraphSchedule>::vertex_iterator>
vertices(const graphir::GraphSchedule& g) {
  return vertices(const_cast<graphir::GraphSchedule&>(g));
}

inline typename boost::graph_traits<graphir::GraphSchedule>::vertices_size_type
num_vertices(graphir::GraphSchedule& g) {
  return const_cast<const graphir::GraphSchedule&>(g).block_size();
}
inline typename boost::graph_traits<graphir::GraphSchedule>::vertices_size_type
num_vertices(const graphir::GraphSchedule& g) {
  return g.block_size();
}

/// IncedenceGraphConcept
inline
std::pair<typename boost::graph_traits<graphir::GraphSchedule>::out_edge_iterator,
          typename boost::graph_traits<graphir::GraphSchedule>::out_edge_iterator>
out_edges(graphir::BasicBlock* u, const graphir::GraphSchedule& g) {
  // for now, we don't care about the kind of edge
  using edge_it_t
    = typename boost::graph_traits<graphir::GraphSchedule>::out_edge_iterator;
  graphir::BasicBlock::EdgeBuilder functor(u);
  return std::make_pair(
    edge_it_t(u->succ_begin(), functor),
    edge_it_t(u->succ_end(), functor)
  );
}

inline
typename boost::graph_traits<graphir::GraphSchedule>::degree_size_type
out_degree(graphir::BasicBlock* u, const graphir::GraphSchedule& g) {
  return u->succ_size();
}

inline
typename boost::graph_traits<graphir::GraphSchedule>::vertex_descriptor
source(const typename boost::graph_traits<graphir::GraphSchedule>::edge_descriptor
       &e,
       const graphir::GraphSchedule& g) {
  return const_cast<graphir::BasicBlock*>(e.first);
}
inline
typename boost::graph_traits<graphir::GraphSchedule>::vertex_descriptor
target(const typename boost::graph_traits<graphir::GraphSchedule>::edge_descriptor
       &e,
       const graphir::GraphSchedule& g) {
  return const_cast<graphir::BasicBlock*>(e.second);
}

/// EdgeListGraphConcept
inline
std::pair<typename boost::graph_traits<graphir::GraphSchedule>::edge_iterator,
          typename boost::graph_traits<graphir::GraphSchedule>::edge_iterator>
edges(graphir::GraphSchedule& g) {
  return std::make_pair(g.edge_begin(), g.edge_end());
}
inline
std::pair<typename boost::graph_traits<graphir::GraphSchedule>::edge_iterator,
          typename boost::graph_traits<graphir::GraphSchedule>::edge_iterator>
edges(const graphir::GraphSchedule& g) {
  return edges(const_cast<graphir::GraphSchedule&>(g));
}

inline typename boost::graph_traits<graphir::GraphSchedule>::edges_size_type
num_edges(graphir::GraphSchedule& g) {
  return const_cast<const graphir::GraphSchedule&>(g).edge_size();
}
inline typename boost::graph_traits<graphir::GraphSchedule>::edges_size_type
num_edges(const graphir::GraphSchedule& g) {
  return g.edge_size();
}
} // end namespace boost

/// Property Map Concept
namespace graphir {
template<>
struct graph_id_map<GraphSchedule, boost::vertex_index_t> {
  using value_type = size_t;
  using reference = size_t;
  using key_type = BasicBlock*;
  struct category : public boost::readable_property_map_tag {};

  graph_id_map(const GraphSchedule& g) : G(g) {}

  reference operator[](const key_type& key) const {
    // just use linear search for now
    // TODO: improve time complexity
    value_type Idx = 0;
    for(auto I = G.block_cbegin(), E = G.block_cend();
        I != E; ++I, ++Idx) {
      if(I->get() == key) break;
    }
    return Idx;
  }

private:
  const GraphSchedule &G;
};
using GraphScheduleVertexIdMap
  = graph_id_map<GraphSchedule, boost::vertex_index_t>;
} // end namespace graphir

namespace boost {
// get() for vertex id property map
inline typename graphir::GraphScheduleVertexIdMap::reference
get(const graphir::GraphScheduleVertexIdMap &pmap,
    const typename graphir::GraphScheduleVertexIdMap::key_type &key) {
  return pmap[key];
}
// get() for getting vertex id property map from graph
inline graphir::GraphScheduleVertexIdMap
get(boost::vertex_index_t tag, const graphir::GraphSchedule& g) {
  return graphir::GraphScheduleVertexIdMap(g);
}
} // end namespace boost
#endif
