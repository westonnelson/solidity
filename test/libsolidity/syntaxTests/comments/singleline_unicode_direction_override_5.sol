// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.7.0;

contract C {
    function f() public pure
    {
        // first push 1, then pop 1
        // ok ‮‬

        // first push 2, then pop 2
        // ok ‮‮‬‬

        // first push 3, then pop 3
        // ok ‮‮‮‬‬‬
    }
}
// ----
