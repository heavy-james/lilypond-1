%%  Do not edit this file; it is auto-generated from LSR!
\version "2.11.35"

\header { texidoc = "
The \\whiteout command underlays a white box under a markup. Since
staff lines are in a lower layer than most other grobs, this white box
will not overlap any other grob.
" }

\paper
{
  ragged-right = ##t
}
{
  \override TextScript #'extra-offset = #'(2 . 4)
  c'4-\markup  { \whiteout \pad-markup #0.5 foo } c
} 
