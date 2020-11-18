// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.7.0;

contract C {
    function f() public pure
    {
        // push 2
        // overflow ‮‮
    }
}
// ----
// ParserError 8936: (138-156): Mismatching directional override markers in comment.
