/*
  inputstaff.hh -- part of LilyPond

  (c) 1996 Han-Wen Nienhuys
*/

#ifndef INPUTSTAFF_HH
#define INPUTSTAFF_HH

#include "string.hh"
#include "plist.hh"
#include "vray.hh"
#include "proto.hh"

struct Input_staff {
    String type;
    IPointerList<Input_command*> commands_;
    IPointerList<Input_music*> music_;

    /****************/

    void add(Input_music*m);
    Input_staff(Input_staff&);
    Input_staff(String);
    void add(svec<Input_command*> &s);
    Staff* parse(PointerList<Input_command*>);
    void print() const;
 
};


#endif // INPUTSTAFF_HH

