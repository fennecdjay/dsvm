# Dead Simple VM

In my current work on [Gwion](https://github.com/Gwion/Gwion),
I want to test the idea of having an Intermediate Representation.

My first try was seemed to feel *stackish*, probably because the current Gwion VM is stack based.

So this is the basis of a register VM I intend to use to design the IR.
It's also probably the basis of what will be used for a register VM backend.

  * Only one integer type.
  * computed gotos
  * a few super instructions

Expect some more work here, in the meantime, feedback in any form very welcome.
