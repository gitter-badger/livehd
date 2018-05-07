
#include <time.h>
#include <string>
#include <random>

#include "bm.h"

#include "inou_rand.hpp"
#include "lgraph.hpp"

Inou_rand::Inou_rand() {

}

Inou_rand_options_pack::Inou_rand_options_pack() {

  assert(Options::get_cargc()!=0); // Options::setup(argc,argv) must be called before setup() is called

  Options::get_desc()->add_options()
    ("rand_seed", boost::program_options::value(&rand_seed), "rand seed value")
    ("rand_type", boost::program_options::value(&rand_type ), "rand graph type to generate")
    ("rand_size", boost::program_options::value(&rand_size ), "rand graph number of nodes")
    ("rand_eratio", boost::program_options::value(&rand_eratio), "rand graph edges/nodes ratio")
    ("rand_crate", boost::program_options::value(&rand_eratio), "percentage of nodes that are constants")
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::command_line_parser(Options::get_cargc(), Options::get_cargv()).options(*Options::get_desc()).allow_unregistered().run(), vm);

  if (vm.count("rand_seed")) {
    rand_seed = vm["rand_seed"].as<int>();
  }else{
    rand_seed = time(0);
  }

  if (vm.count("rand_size")) {
    rand_size = vm["rand_size"].as<int>();
  }else{
    rand_size = 1024;
  }

  if (vm.count("rand_type")) {
    rand_type = vm["rand_type"].as<std::string>();
  }else{
    rand_type = "netlist";
  }

  if (rand_type == "netlist") {
    rand_eratio = 3.2;
  }else{
    rand_eratio = 3.2;
  }

  if (vm.count("rand_eratio"))
    rand_eratio = vm["rand_eratio"].as<double>();

  if (rand_type == "rand_crate") {
    rand_crate = vm["rand_crate"].as<int>();
  }else{
    rand_crate = 10;
  }

  console->info("inou_rand rand_seed={} rand_size={} rand_type={} rand_eratio={}", rand_seed, rand_size, rand_type, rand_eratio);
}

struct pin_pair_compare {
  bool operator() (const std::pair<Node_Pin,Node_Pin>& lhs, const std::pair<Node_Pin,Node_Pin>& rhs) const {
    if(lhs.first.get_nid() < rhs.first.get_nid())
      return true;

    if(lhs.first.get_nid() == rhs.first.get_nid() && lhs.first.get_pid() < rhs.first.get_pid())
      return true;

    if(lhs.first.get_nid() == rhs.first.get_nid() && lhs.first.get_pid() < rhs.first.get_pid() &&
        lhs.second.get_nid() < rhs.second.get_nid())
      return true;

    if(lhs.first.get_nid() == rhs.first.get_nid() && lhs.first.get_pid() < rhs.first.get_pid() &&
        lhs.second.get_nid() == rhs.second.get_nid() && lhs.second.get_pid() < rhs.second.get_pid())
      return true;

    return false;
  }
};

std::vector<LGraph *> Inou_rand::generate() {

  std::vector<LGraph *> lgs;

  auto *g = new LGraph(opack.lgdb_path);

  std::mt19937 rnd;
  rnd.seed(opack.rand_seed);

  std::uniform_int_distribution<Index_ID> rnd_created(0,opack.rand_size-1);
  std::uniform_int_distribution<Port_ID>  rnd_4(0,4);
  std::uniform_int_distribution<uint16_t> rnd_bits1(1,32);
  std::uniform_int_distribution<uint16_t> rnd_bits2(1,512);
  std::uniform_int_distribution<uint8_t>  rnd_op(Sum_Op,SubGraph_Op-1);
  std::uniform_int_distribution<uint32_t> rnd_u32op(0,(uint32_t)(U32ConstMax_Op-U32ConstMin_Op));
  std::uniform_int_distribution<uint8_t>  rnd_const(0,100);

  std::vector<Index_ID> created;

  Index_ID max_nid=0;
  for(int i=0;i<opack.rand_size;i++) {
    Node node = g->create_node();
    Index_ID nid = node.get_nid();
    created.push_back(nid);
    if (nid>max_nid)
      max_nid = nid;

    if (rnd_const(rnd) < opack.rand_crate) {
      g->node_u32type_set(nid, static_cast<Node_Type_Op>(rnd_u32op(rnd)));
    } else {
      g->node_type_set(nid, static_cast<Node_Type_Op>(rnd_op(rnd)));
    }
  }

  bm::bvector<> used_port;

  std::set<std::pair<Node_Pin, Node_Pin>, struct pin_pair_compare> connections;

  int i = 0;
  int timeout = 0;
  while(i < opack.rand_eratio*opack.rand_size) {
    uint16_t rbits = 1;
    switch (rnd_4(rnd)) {
      case 0: rbits = 1;              break;
      case 1: rbits = 1<<rnd_4(rnd);  break;
      case 2: rbits = rnd_bits2(rnd); break;
      default: rbits = rnd_bits1(rnd);
    }
    Index_ID dst_nid = created[rnd_created(rnd)];
    Node_Type dst_type = g->node_type_get(dst_nid);
    //if constant, we don't allow inputs to node
    if(dst_type.op > U32Const_Op && dst_type.op <= U32ConstMax_Op)
      continue;

    Port_ID dst_port = 0;
    if (used_port.get_bit(dst_nid)) {
      dst_port =rnd_4(rnd);
    }
    used_port.set_bit(dst_nid);

    Index_ID src_nid = created[rnd_created(rnd)];

    Node_Pin dst_pin(dst_nid , dst_port, true);
    Node_Pin src_pin(src_nid, rnd_4(rnd), false);

    //prevent adding same edge twice
    std::pair<Node_Pin,Node_Pin> conn(src_pin, dst_pin);
    if(connections.find(conn) == connections.end()) {
      g->add_edge(src_pin, dst_pin, rbits);
      connections.insert(conn);
      i++;
      timeout = 0;
      std::cout << "added, ";
    } else {
      timeout++;
    }
    std::cout << " checking " << (connections.find(conn) == connections.end()) << std::endl;

    if(timeout > 10000) {
      std::cout << "I was not able to add all the connections, giving up due to timeout." << std::endl;
      break;
    }

  }

  g->sync();

  lgs.push_back(g);

  return lgs;
}

void Inou_rand::generate(std::vector<const LGraph *> out) {

  assert(0); // No method to randinly transform a graph, just to generate.

  out.clear();
}
