//  This file is distributed under the BSD 3-Clause License. See LICENSE for details.
#pragma once

#include "pass.hpp"
#include "lnast.hpp"
#include "lgraph.hpp"

class Pass_lgraph_to_lnast : public Pass {
protected:
  std::vector<Lnast> lnasts;
  uint64_t           temp_var_count = 0;

  static void trans(Eprp_var &var);
  void        do_trans(LGraph *g);

  bool        iterate_over_lg(LGraph *g);
  void        initial_tree_coloring(LGraph *g);
  void        begin_transformation(LGraph *g, Lnast& lnast, Lnast_nid& ln_node);
  void        handle_output_node(LGraph *lg, Node_pin& pin, Lnast& lnast, Lnast_nid& ln_node);
  void        handle_source_node(LGraph *lg, Node_pin& pin, Lnast& lnast, Lnast_nid& ln_node);

  void        attach_to_lnast(Lnast& lnast, Lnast_nid& parent_node, const Node_pin &pin);
  void        attach_sum_node(Lnast& lnast, Lnast_nid& parent_node, const Node_pin &pin);

  void handle_io(LGraph *g, Lnast_nid& parent_lnast_node, Lnast& lnast);

public:
  Pass_lgraph_to_lnast(const Eprp_var &var);

  static void setup();
};
