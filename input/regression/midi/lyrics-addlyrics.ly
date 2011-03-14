% Lily was here -- automatically converted by ../../../scripts/midi2ly.py from out/initial-key.midi
\version "2.13.53"

\layout {
  \context {
    \Voice
    \remove "Note_heads_engraver"
    \consists "Completion_heads_engraver"
    \remove "Rest_engraver"
    \consists "Completion_rest_engraver"
  }
}

% included from ./out/initial-key.header
\header {
texidoc="Lyrics are preserved"
options=""
}
% end

\score {
  <<
    \relative c'' {
      \key g \major
      \time 6/8
      d4 b8 c4 a8 | d4 b8 g4
    }
    \addlyrics {
      Girls and boys come | out to play,
    }
  >>
  \layout {}
  \midi {}
}
