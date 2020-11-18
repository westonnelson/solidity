// SPDX-License-Identifier: UNLICENSED
pragma solidity ^0.7.0;

contract C {
    function f() public pure
    {
        // pop 2
        /*underflow ‬‬*/
    }
}
// ----
// ParserError 8936: (137-152): Unicode direction override underflow in comment.
