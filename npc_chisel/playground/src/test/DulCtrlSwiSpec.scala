package dcs

import chisel3._ 
import chisel3.experimental.BundleLiterals._
import chisel3.simulator.EphemeralSimulator._
import org.scalatest.freespec.AnyFreeSpec
import org.scalatest.matchers.must.Matchers

class DulCtrlSwiSpec extends AnyFreeSpec with Matchers {
    "DulCtrlSwiSpec should react correctly" in {
        simulate(new DulCtrlSwi()) { dut => 
            for(a <- 0 to 1) {
                for(b <- 0 to 1) {
                    dut.io.a.poke(a.U)
                    dut.io.b.poke(b.U)
                    dut.clock.step()
                    val res = a ^ b
                    dut.io.out.expect(res.U)
                }
            } 
            
        }
    }
}