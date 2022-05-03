package boom.exu

import chisel3._
import chisel3.util._

import freechips.rocketchip.config.Parameters
import freechips.rocketchip.rocket._

import boom.common._
import boom.util._


class EventCounterIO(readWidth: Int)(implicit p: Parameters) extends BoomBundle
{
  val event_signals_l  = Input(Vec(16, UInt(4.W)))
  val event_signals_h  = Input(Vec(16, UInt(4.W)))

  val read_addr = Input(Vec(readWidth, Valid(UInt(5.W))))
  val read_data = Output(Vec(readWidth, UInt(64.W)))

  val reset_counter = Input(Bool())
}


class EventCounter(readWidth: Int)(implicit p: Parameters) extends BoomModule
{
	val io = IO(new EventCounterIO(readWidth))
	val reg_counters = io.event_signals_l.zipWithIndex.map { case (e, i) => freechips.rocketchip.util.WideCounter(64, e, reset = false) }
	val reg_counters_high = io.event_signals_h.zipWithIndex.map { case (e, i) => freechips.rocketchip.util.WideCounter(64, e, reset = false) }

	//for reg counter value
	val reg_read_data = Reg(Vec(readWidth, UInt(64.W)))
	for(i <- 0 until readWidth){
		io.read_data(i) := RegNext(reg_read_data(i))
	}

  for (i <- 0 until readWidth) {
    when (io.read_addr(i).valid) {
      for (w <- 0 until 16) {
        when (io.read_addr(i).bits(3,0) === w.U) {
          reg_read_data(i) := Mux(io.read_addr(i).bits(4) === 0.U, reg_counters(w), reg_counters_high(w))
        }
      }
    }
    .otherwise {
      reg_read_data(i) := 0.U
    }
  }

  when (io.reset_counter) {
    for (w <- 0 until 16) {
      reg_counters(w) := 0.U
      reg_counters_high(w) := 0.U
    }
  }

  when (RegNext(io.reset_counter)) {
    for (w <- 0 until 16) {
      printf("w: %d, counter: %d  %d\n", w.U, reg_counters(w), reg_counters_high(w) )
    }
  }
}
