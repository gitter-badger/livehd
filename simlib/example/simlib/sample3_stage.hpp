#pragma once
#include "vcd_writer.hpp"
#include <time.h>
struct Sample3_stage {
  uint64_t hidx;

  UInt<32> to1_b;

  std::array<UInt<32>, 256> memory;

  uint8_t reset_iterator;
  UInt<32> tmp;
  UInt<32> tmp2;

  #ifdef SIMLIB_VCD
  vcd::VCDWriter vcd_writer{"SIMLIB_VCD.vcd", vcd::makeVCDHeader(vcd::TimeScale::ONE, vcd::TimeScaleUnit::ns, vcd::utils::now(), "This is the VCD file", "version_simlib_") };
  clock_t t;// = clock();

  vcd::VarPtr vcd_to1_b = vcd_writer.register_var("SS3", "to1_b", vcd::VariableType::integer, sizeof(to1_b));
    vcd::VarPtr vcd_tmp = vcd_writer.register_var("SS3", "tmp", vcd::VariableType::integer, sizeof(to1_b));
    vcd::VarPtr vcd_tmp2 = vcd_writer.register_var("SS3", "tmp2", vcd::VariableType::integer, sizeof(to1_b));
    void vcd_cycle(UInt<1> s1_to3_cValid, UInt<32> s1_to3_c, UInt<1> s2_to3_dValid, UInt<32> s2_to3_d);
  #endif

    Sample3_stage(uint64_t _hidx);

  void reset_cycle();
  void cycle(UInt<1> s1_to3_cValid, UInt<32> s1_to3_c, UInt<1> s2_to3_dValid, UInt<32> s2_to3_d);
  void add_signature(Simlib_signature &sign);
};

