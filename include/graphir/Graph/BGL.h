#ifndef GRAPHIR_GRAPH_BGL_H
#define GRAPHIR_GRAPH_BGL_H

/// Defining required traits for boost graph library
#include <iostream>
#include <utility>

#include "boost/graph/graph_traits.hpp"
#include "boost/graph/properties.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include "boost/property_map/property_map.hpp"
#include "graphir/Graph/Graph.h"
#include "graphir/Graph/Node.h"
#include "graphir/Graph/NodeUtils.h"
#include "graphir/Support/STLExtras.h"
#include "graphir/Support/type_traits.h"

namespace boost {
template <>
struct graph_traits<graphir::Graph> {
  /// GraphConcept
  using vertex_descriptor = graphir::Node*;
  using edge_descriptor = graphir::Use;
  using directed_category = boost::directed_tag;
  using edge_parallel_category = boost::allow_parallel_edge_tag;

  static vertex_descriptor null_vertex() { return nullptr; }

  /// VertexListGraphConcept
  using vertex_iterator =
      boost::transform_iterator<graphir::unique_ptr_unwrapper<graphir::Node>,
                                typename graphir::Graph::node_iterator,
                                graphir::Node*,  // Refrence type
                                graphir::Node*   // Value type
                                >;
  using vertices_size_type = size_t;

  /// EdgeListGraphConcept
  using edges_size_type = size_t;
  using edge_iterator = typename graphir::Graph::edge_iterator;

  /// IncidenceGraphConcept
  using out_edge_iterator =
      boost::transform_iterator<typename graphir::Use::BuilderFunctor,
                                typename graphir::Node::input_iterator,
                                graphir::Use,  // Reference type
                                graphir::Use   // Value type
                                >;
  using degree_size_type = size_t;

  struct traversal_category : public boost::vertex_list_graph_tag,
                              public boost::edge_list_graph_tag,
                              public boost::incidence_graph_tag {};
};

template <>
struct graph_traits<graphir::SubGraph> {
  /// GraphConcept
  using vertex_descriptor = graphir::Node*;
  using edge_descriptor = graphir::Use;
  using directed_category = boost::directed_tag;
  using edge_parallel_category = boost::allow_parallel_edge_tag;

  static vertex_descriptor null_vertex() { return nullptr; }

  /// VertexListGraphConcept
  using vertex_iterator = typename graphir::SubGraph::node_iterator;
  using vertices_size_type = size_t;

  /// EdgeListGraphConcept
  using edges_size_type = size_t;
  using edge_iterator = typename graphir::SubGraph::edge_iterator;

  /// IncidenceGraphConcept
  using out_edge_iterator =
      boost::transform_iterator<typename graphir::Use::BuilderFunctor,
                                typename graphir::Node::input_iterator,
                                graphir::Use,  // Reference type
                                graphir::Use   // Value type
                                >;
  using degree_size_type = size_t;

  struct traversal_category : public boost::vertex_list_graph_tag,
                              public boost::edge_list_graph_tag,
                              public boost::incidence_graph_tag {};
};

/// Note: We mark most of the BGL trait functions here as inline
/// because they're trivial.
/// FIXME: Will putting them into separated source file helps reducing
/// compilation time?

/// VertexListGraphConcept
inline std::pair<typename boost::graph_traits<graphir::Graph>::vertex_iterator,
                 typename boost::graph_traits<graphir::Graph>::vertex_iterator>
vertices(graphir::Graph& g) {
  using vertex_it_t =
      typename boost::graph_traits<graphir::Graph>::vertex_iterator;
  graphir::unique_ptr_unwrapper<graphir::Node> functor;
  return std::make_pair(vertex_it_t(g.node_begin(), functor),
                        vertex_it_t(g.node_end(), functor));
}
inline std::pair<typename boost::graph_traits<graphir::Graph>::vertex_iterator,
                 typename boost::graph_traits<graphir::Graph>::vertex_iterator>
vertices(const graphir::Graph& g) {
  return vertices(const_cast<graphir::Graph&>(g));
}
// SubGraph
inline std::pair<
    typename boost::graph_traits<graphir::SubGraph>::vertex_iterator,
    typename boost::graph_traits<graphir::SubGraph>::vertex_iterator>
vertices(graphir::SubGraph& g) {
  return std::make_pair(g.node_begin(), g.node_end());
}
inline std::pair<
    typename boost::graph_traits<graphir::SubGraph>::vertex_iterator,
    typename boost::graph_traits<graphir::SubGraph>::vertex_iterator>
vertices(const graphir::SubGraph& g) {
  return vertices(const_cast<graphir::SubGraph&>(g));
}

inline typename boost::graph_traits<graphir::Graph>::vertices_size_type
num_vertices(graphir::Graph& g) {
  return const_cast<const graphir::Graph&>(g).node_size();
}
inline typename boost::graph_traits<graphir::Graph>::vertices_size_type
num_vertices(const graphir::Graph& g) {
  return g.node_size();
}
// SubGraph
inline typename boost::graph_traits<graphir::SubGraph>::vertices_size_type
num_vertices(graphir::SubGraph& g) {
  return const_cast<const graphir::SubGraph&>(g).node_size();
}
inline typename boost::graph_traits<graphir::SubGraph>::vertices_size_type
num_vertices(const graphir::SubGraph& g) {
  return g.node_size();
}

/// EdgeListGraphConcept
inline std::pair<typename boost::graph_traits<graphir::Graph>::edge_iterator,
                 typename boost::graph_traits<graphir::Graph>::edge_iterator>
edges(graphir::Graph& g) {
  return std::make_pair(g.edge_begin(), g.edge_end());
}
inline std::pair<typename boost::graph_traits<graphir::Graph>::edge_iterator,
                 typename boost::graph_traits<graphir::Graph>::edge_iterator>
edges(const graphir::Graph& g) {
  return edges(const_cast<graphir::Graph&>(g));
}
// SubGraph
inline std::pair<typename boost::graph_traits<graphir::SubGraph>::edge_iterator,
                 typename boost::graph_traits<graphir::SubGraph>::edge_iterator>
edges(graphir::SubGraph& g) {
  return std::make_pair(g.edge_begin(), g.edge_end());
}
inline std::pair<typename boost::graph_traits<graphir::SubGraph>::edge_iterator,
                 typename boost::graph_traits<graphir::SubGraph>::edge_iterator>
edges(const graphir::SubGraph& g) {
  return edges(const_cast<graphir::SubGraph&>(g));
}

inline typename boost::graph_traits<graphir::Graph>::edges_size_type num_edges(
    graphir::Graph& g) {
  return const_cast<const graphir::Graph&>(g).edge_size();
}
inline typename boost::graph_traits<graphir::Graph>::edges_size_type num_edges(
    const graphir::Graph& g) {
  return g.edge_size();
}
// SubGraph
inline typename boost::graph_traits<graphir::SubGraph>::edges_size_type
num_edges(graphir::SubGraph& g) {
  return const_cast<const graphir::SubGraph&>(g).edge_size();
}
inline typename boost::graph_traits<graphir::SubGraph>::edges_size_type
num_edges(const graphir::SubGraph& g) {
  return g.edge_size();
}

template <class GraphT>
inline graphir::enable_if_t<
    std::is_same<GraphT, graphir::Graph>::value ||
        std::is_same<GraphT, graphir::SubGraph>::value,
    typename boost::graph_traits<GraphT>::vertex_descriptor>
source(const graphir::Use& e, const GraphT& g) {
  return const_cast<graphir::Node*>(e.source);
}
template <class GraphT>
inline graphir::enable_if_t<
    std::is_same<GraphT, graphir::Graph>::value ||
        std::is_same<GraphT, graphir::SubGraph>::value,
    typename boost::graph_traits<GraphT>::vertex_descriptor>
target(const graphir::Use& e, const GraphT& g) {
  return const_cast<graphir::Node*>(e.dest);
}

/// IncidenceGraphConcept
template <class T>
inline graphir::enable_if_t<
    std::is_same<T, graphir::Graph>::value ||
        std::is_same<T, graphir::SubGraph>::value,
    std::pair<typename boost::graph_traits<T>::out_edge_iterator,
              typename boost::graph_traits<T>::out_edge_iterator>>
out_edges(graphir::Node* u, const T& g) {
  // for now, we don't care about the kind of edge
  using edge_it_t = typename boost::graph_traits<T>::out_edge_iterator;
  graphir::Use::BuilderFunctor functor(u, graphir::Use::K_NONE,
                                       g.GetEdgePatcher());
  return std::make_pair(edge_it_t(u->inputs().begin(), functor),
                        edge_it_t(u->inputs().end(), functor));
}

template <class T>
inline graphir::enable_if_t<std::is_same<T, graphir::Graph>::value ||
                                std::is_same<T, graphir::SubGraph>::value,
                            typename boost::graph_traits<T>::degree_size_type>
out_degree(graphir::Node* u, const T& g) {
  return u->getNumValueInput() + u->getNumControlInput() +
         u->getNumEffectInput();
}
}  // end namespace boost

/// Property Map Concept
namespace graphir {
template <class GraphT>
struct graph_id_map<GraphT, boost::vertex_index_t> {
  using value_type = size_t;
  using reference = size_t;
  using key_type = Node*;
  struct category : public boost::readable_property_map_tag {};

  graph_id_map(const GraphT& g) : G(g) {}

  reference operator[](const key_type& key) const {
    // just use linear search for now
    // TODO: improve time complexity
    value_type Idx = 0;
    for (auto I = G.node_cbegin(), E = G.node_cend(); I != E; ++I, ++Idx) {
      if (GraphT::GetNodeFromIt(I) == key)
        break;
    }
    return Idx;
  }

 private:
  const GraphT& G;
};
}  // end namespace graphir

namespace boost {
// get() for vertex id property map
template <class GraphT>
inline graphir::enable_if_t<
    std::is_same<GraphT, graphir::Graph>::value ||
        std::is_same<GraphT, graphir::SubGraph>::value,
    typename graphir::graph_id_map<GraphT, boost::vertex_index_t>::reference>
get(const typename graphir::graph_id_map<GraphT, boost::vertex_index_t>& pmap,
    const typename graphir::graph_id_map<
        GraphT, boost::vertex_index_t>::key_type& key) {
  return pmap[key];
}
// get() for getting vertex id property map from graph
template <class GraphT>
inline graphir::enable_if_t<
    std::is_same<GraphT, graphir::Graph>::value ||
        std::is_same<GraphT, graphir::SubGraph>::value,
    typename graphir::graph_id_map<GraphT, boost::vertex_index_t>>
get(boost::vertex_index_t tag, const GraphT& g) {
  return graphir::graph_id_map<GraphT, boost::vertex_index_t>(g);
}
}  // end namespace boost

/// PropertyWriter Concept
namespace graphir {
struct graph_prop_writer {
  void operator()(std::ostream& OS) const {
    // print the graph 'upside down'
    OS << "rankdir = BT;" << std::endl;
  }
};

struct graph_vertex_prop_writer {
  graph_vertex_prop_writer(const Graph& g) : G(g) {}

  void operator()(std::ostream& OS, const Node* v) const {
    Node* N = const_cast<Node*>(v);
    OS << "[label=\"";
    IrOpcode::Print(G, OS, N);
    OS << "\"]";
  }

 private:
  const Graph& G;
};

struct graph_edge_prop_writer {
  void operator()(std::ostream& OS, const Use& U) const {
    switch (U.dep_kind) {
      case Use::K_VALUE:
        OS << "[color=\"black\"]";
        break;
      case Use::K_CONTROL:
        OS << "[color=\"blue\"]";
        break;
      case Use::K_EFFECT:
        OS << "[color=\"red\", style=\"dashed\"]";
        break;
      default:
        graphir_unreachable("Invalid edge kind");
    }
  }
};

}  // end namespace graphir

#endif  // GRAPHIR_GRAPH_BGL_H