\version "2.1.26"

\header {
texidoc= "@cindex make-music Fingering
You can add various stuff to notes using @code{make-music}.
In this example, an extra fingering is attached to a note. 

In general, first do a @code{display} of the music you want to
create, then write a function that will structure the music for you.
"
} 

#(define (make-text-script x) 
   (let ((m (make-music-by-name 'TextScriptEvent)))
    (ly:music-set-property! m 'direction DOWN) 
     (ly:music-set-property! m 'text (make-simple-markup x))
     m))
     
#(define (add-text-script m x)
   (if (equal? (ly:music-property m 'name) 'EventChord)
       (ly:music-set-property! m 'elements
			    (cons (make-text-script x)
				  (ly:music-property m 'elements)))
       
       (let ((es (ly:music-property m 'elements))
	     (e (ly:music-property m 'element)) )
	 (map (lambda (y) (add-text-script y x)) es)
	 (if (ly:music? e)
	     (add-text-script e x))))
   m)

\score {
  \apply #(lambda (x)  (add-text-script x "6") (display-music x) x ) \notes { c'4-3 }
	\paper{ raggedright = ##t }
}


