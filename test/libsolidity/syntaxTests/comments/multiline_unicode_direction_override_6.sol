// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.7.0;

contract C {
    function f() public pure
    {
        // first pop, then push
        /*overflow ‬‮*/
    }
}
// ----
// ParserError 8936: (152-166): Unicode direction override underflow in comment.
