#include "graphir/Graph/BGL.h"

#include <gtest/gtest.h>

#include <boost/concept/assert.hpp>
#include <boost/graph/graph_concepts.hpp>

#include "graphir/Graph/Graph.h"

TEST(GraphBGLUnitTest, TestGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::GraphConcept<graphir::Graph>));
}
TEST(GraphBGLUnitTest, TestVertexListGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<graphir::Graph>));
}
TEST(GraphBGLUnitTest, TestEdgeListGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::EdgeListGraphConcept<graphir::Graph>));
}
TEST(GraphBGLUnitTest, TestIncidenceGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<graphir::Graph>));
}

TEST(GraphBGLUnitTest, TestVertexReadablePropertyMapConcept) {
  BOOST_CONCEPT_ASSERT(
      (boost::ReadablePropertyMapConcept<
          graphir::graph_id_map<graphir::Graph, boost::vertex_index_t>,
          graphir::Node*>));
}

TEST(SubGraphBGLUnitTest, TestGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::GraphConcept<graphir::SubGraph>));
}
TEST(SubGraphBGLUnitTest, TestVertexListGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept<graphir::SubGraph>));
}
TEST(SubGraphBGLUnitTest, TestEdgeListGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::EdgeListGraphConcept<graphir::SubGraph>));
}
TEST(SubGraphBGLUnitTest, TestIncidenceGraphConcept) {
  BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept<graphir::SubGraph>));
}

TEST(SubGraphBGLUnitTest, TestVertexReadablePropertyMapConcept) {
  BOOST_CONCEPT_ASSERT(
      (boost::ReadablePropertyMapConcept<
          graphir::graph_id_map<graphir::SubGraph, boost::vertex_index_t>,
          graphir::Node*>));
}