// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.7.0;

contract C {
    function f() public pure
    {
        // first pop, then push
        // underflow ‬‮
    }
}
// ----
// ParserError 8936: (152-168): Unicode direction override underflow in comment.
