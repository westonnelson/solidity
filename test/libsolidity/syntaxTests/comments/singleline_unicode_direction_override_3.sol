// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.7.0;

contract C {
    function f() public pure
    {
        // push 1
        // overflow ‮
    }
}
// ----
// ParserError 8936: (138-153): Mismatching directional override markers in comment.
