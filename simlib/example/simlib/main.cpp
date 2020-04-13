//  This file is distributed under the BSD 3-Clause License. See LICENSE for details.
#include "livesim_types.hpp"
#include "simlib_checkpoint.hpp"
#include "sample_stage.hpp"
#include "vcd_writer.hpp"
#include "vcd_utils.cpp"

int main(int argc, char **argv) {
  Simlib_checkpoint<Sample_stage> top("ckpt");
  top.enable_trace(".");
//  vcd::VCDWriter vcd_writer{"SIMLIB_VCD.vcd", vcd::makeVCDHeader(vcd::TimeScale::ONE, vcd::TimeScaleUnit::ns, vcd::utils::now(), "This is the VCD file", "version_simlib_") };
  top.advance_clock(100000000);
  // Replay last cycles:
  //top.load_intermediate_checkpoint(30000);
  //top.advance_clock(100000000-30000);
  // top.advance_clock(100000000-30000);
  //auto v = top.find_previous_checkpoint(3000000);
  //printf("previous_check = %lld\n", (long)v);
  return 0;
}

