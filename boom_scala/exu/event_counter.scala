package boom.exu

import chisel3._
import chisel3.util._

import freechips.rocketchip.config.Parameters
import freechips.rocketchip.rocket._

import boom.common._
import boom.util._

//Enable_PerfCounter_Support
class SubEventCounterIO(readWidth: Int)(implicit p: Parameters) extends BoomBundle
{
  val event_signals  = Input(Vec(16, UInt(4.W)))
  val read_addr = Input(Vec(readWidth, Valid(UInt(4.W))))
  val read_data = Output(Vec(readWidth, UInt(64.W)))
  val reset_counter = Input(Bool())
}

class SubEventCounter(readWidth: Int)(implicit p: Parameters) extends BoomModule
{
	val io = IO(new SubEventCounterIO(readWidth))
	val reg_counters = io.event_signals.zipWithIndex.map { case (e, i) => freechips.rocketchip.util.WideCounter(64, e, reset = false) }

  for (i <- 0 until readWidth) {
    when (io.read_addr(i).valid) {
      for (w <- 0 until 16) {
        when (io.read_addr(i).bits === w.U) {
          io.read_data(i) := reg_counters(w)
        }
      }
    }
    .otherwise {
      io.read_data(i) := 0.U
    }
  }

  when (io.reset_counter) {
    for (w <- 0 until 16) {
      reg_counters(w) := 0.U
    }
  }

  when (RegNext(io.reset_counter)) {
    for (w <- 0 until 16) {
      printf("w: %d, counter: %d\n", w.U, reg_counters(w) )
    }
  }

}


class EventCounterIO(readWidth: Int, signalnum: Int)(implicit p: Parameters) extends BoomBundle
{
  val event_signals  = Input(Vec(signalnum, UInt(4.W)))
  val read_addr = Input(Vec(readWidth, Valid(UInt(8.W))))
  val read_data = Output(Vec(readWidth, UInt(64.W)))
  val reset_counter = Input(Bool())
}


class EventCounter(readWidth: Int)(implicit p: Parameters) extends BoomModule
{
  
	val io = IO(new EventCounterIO(readWidth, 16*subECounterNum))

  val ecounters = for (w <- 0 until subECounterNum) yield { val e = Module(new SubEventCounter(readWidth)); e }

  //for two cycles delay
	val reg_read_data = Reg(Vec(readWidth, UInt(64.W)))
	for(i <- 0 until readWidth){
		io.read_data(i) := RegNext(reg_read_data(i))
	}

  for (w <- 0 until subECounterNum) {
    ecounters(w).io.reset_counter := io.reset_counter
    for (s <- 0 until 16) {
      ecounters(w).io.event_signals(s) := io.event_signals(s+16*w)
    }

    for (r <- 0 until readWidth) {
      val tag = io.read_addr(r).bits(7, 4)
      ecounters(w).io.read_addr(r).valid := io.read_addr(r).valid && (tag === w.U)
      ecounters(w).io.read_addr(r).bits  := io.read_addr(r).bits(3, 0)
    }
  }

  for (i <- 0 until readWidth) {
    when (io.read_addr(i).valid) {
      val tag = io.read_addr(i).bits(7, 4)
      for (w <- 0 until subECounterNum) {
        when (w.U === tag) {
          reg_read_data(i) := ecounters(w).io.read_data(i)
        }
      }
    }
    .otherwise {
      reg_read_data(i) := 0.U
    }
  }
}
