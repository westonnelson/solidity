object "A" {
  code {
    pop(dataoffset("A"))
    pop(datasize("A"))
    pop(dataoffset("B"))
    pop(datasize("B"))
    pop(dataoffset("B.C"))
    pop(datasize("B.C"))
    pop(dataoffset("B.E"))
    pop(datasize("B.E"))
    pop(dataoffset("B.C.D"))
    pop(datasize("B.C.D"))
  }

  data "data1" "Hello, World!"

  object "B" {
    code {
      pop(dataoffset("C"))
      pop(datasize("C"))
      pop(dataoffset("E"))
      pop(datasize("E"))
      pop(dataoffset("C.D"))
      pop(datasize("C.D"))
    }
    object "C" {
      code {
        pop(dataoffset("D"))
        pop(datasize("D"))
      }
      object "D" {
        code {
          invalid()
        }
      }
    }
    object "E" {
      code {
        invalid()
      }
    }
  }
}
// ----
// Assembly:
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//     stop
//
//     sub_0: assembly {
//         stop
//
//         sub_0: assembly {
//                 /* "source":645:654   */
//               invalid
//         }
//     }
//
//     sub_1: assembly {
//             /* "source":717:726   */
//           invalid
//     }
// }
// Bytecode: fe
// Opcodes: INVALID
// SourceMappings:
