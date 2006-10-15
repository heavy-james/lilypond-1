/*
  beam-engraver.cc -- implement Beam_engraver

  source file of the GNU LilyPond music typesetter

  (c) 1998--2006 Han-Wen Nienhuys <hanwen@xs4all.nl>
*/

#include "beam.hh"
#include "beaming-pattern.hh"
#include "context.hh"
#include "drul-array.hh"
#include "duration.hh"
#include "engraver.hh"
#include "international.hh"
#include "item.hh"
#include "rest.hh"
#include "spanner.hh"
#include "stream-event.hh"
#include "stem.hh"
#include "warn.hh"

#include "translator.icc"

class Beam_engraver : public Engraver
{
public:
  DECLARE_ACKNOWLEDGER (stem);
  DECLARE_ACKNOWLEDGER (rest);
protected:
  Stream_event *start_ev_;

  Spanner *finished_beam_;
  Spanner *beam_;
  Stream_event *prev_start_ev_;

  Stream_event *now_stop_ev_;

  Beaming_pattern *beam_info_;
  Beaming_pattern *finished_beam_info_;

  /// location  within measure where beam started.
  Moment beam_start_location_;

  /// moment (global time) where beam started.
  Moment beam_start_mom_;

  bool subdivide_beams_;
  Moment beat_length_;

  void typeset_beam ();
  void set_melisma (bool);

  Moment last_stem_added_at_;
  void stop_translation_timestep ();
  void start_translation_timestep ();
  virtual void finalize ();

  void process_music ();

  virtual bool valid_start_point ();
  virtual bool valid_end_point ();

  DECLARE_TRANSLATOR_LISTENER (beam);
public:
  TRANSLATOR_DECLARATIONS (Beam_engraver);
};

/*
  Hmm. this isn't necessary, since grace beams and normal beams are
  always nested.
*/
bool
Beam_engraver::valid_start_point ()
{
  Moment n = now_mom ();

  return n.grace_part_ == Rational (0);
}

bool
Beam_engraver::valid_end_point ()
{
  return valid_start_point ();
}

Beam_engraver::Beam_engraver ()
{
  beam_ = 0;
  finished_beam_ = 0;
  finished_beam_info_ = 0;
  beam_info_ = 0;
  now_stop_ev_ = 0;
  start_ev_ = 0;
  prev_start_ev_ = 0;
}

IMPLEMENT_TRANSLATOR_LISTENER (Beam_engraver, beam);
void
Beam_engraver::listen_beam (Stream_event *ev)
{
  Direction d = to_dir (ev->get_property ("span-direction"));

  if (d == START && valid_start_point ())
    ASSIGN_EVENT_ONCE (start_ev_, ev);
  else if (d == STOP && valid_end_point ())
    ASSIGN_EVENT_ONCE (now_stop_ev_, ev);
}

void
Beam_engraver::set_melisma (bool ml)
{
  SCM b = get_property ("autoBeaming");
  if (!to_boolean (b))
    context ()->set_property ("beamMelismaBusy", ml ? SCM_BOOL_T : SCM_BOOL_F);
}

void
Beam_engraver::process_music ()
{
  if (beam_ && !to_boolean (get_property ("allowBeamBreak")))
    context ()->get_score_context ()->set_property ("forbidBreak", SCM_BOOL_T);

  if (start_ev_)
    {
      if (beam_)
	{
	  start_ev_->origin ()->warning (_ ("already have a beam"));
	  return;
	}

      set_melisma (true);
      prev_start_ev_ = start_ev_;
      beam_ = make_spanner ("Beam", start_ev_->self_scm ());
      Moment mp (robust_scm2moment (get_property ("measurePosition"), Moment (0)));

      beam_start_location_ = mp;
      beam_start_mom_ = now_mom ();

      beam_info_ = new Beaming_pattern;
      /* urg, must copy to Auto_beam_engraver too */
    }
}

void
Beam_engraver::typeset_beam ()
{
  if (finished_beam_)
    {
      if (!finished_beam_->get_bound (RIGHT))
	finished_beam_->set_bound (RIGHT, finished_beam_->get_bound (LEFT));
	  
      finished_beam_info_->beamify (context ());
      Beam::set_beaming (finished_beam_, finished_beam_info_);

      delete finished_beam_info_;
      finished_beam_info_ = 0;
      finished_beam_ = 0;
    }
}

void
Beam_engraver::start_translation_timestep ()
{
  start_ev_ = 0;

  if (beam_)
    {
      set_melisma (true);
    }
}

void
Beam_engraver::stop_translation_timestep ()
{
  typeset_beam ();
  if (now_stop_ev_)
    {
      finished_beam_ = beam_;
      finished_beam_info_ = beam_info_;

      now_stop_ev_ = 0;
      beam_ = 0;
      beam_info_ = 0;
      typeset_beam ();
      set_melisma (false);
    }
}

void
Beam_engraver::finalize ()
{
  typeset_beam ();
  if (beam_)
    {
      prev_start_ev_->origin ()->warning (_ ("unterminated beam"));

      /*
	we don't typeset it, (we used to, but it was commented
	out. Reason unknown) */
      beam_->suicide ();
      delete beam_info_;
    }
}

void
Beam_engraver::acknowledge_rest (Grob_info info)
{
  if (beam_)
    {
      chain_offset_callback (info.grob(),
			     Beam::rest_collision_callback_proc, Y_AXIS);
    }
}



void
Beam_engraver::acknowledge_stem (Grob_info info)
{
  if (!beam_)
    return;
  
  Moment now = now_mom ();
  if (!valid_start_point ())
    return;

  Item *stem = dynamic_cast<Item *> (info.grob ());
  if (Stem::get_beam (stem))
    return;

  
  
  Stream_event *ev = info.ultimate_event_cause ();
  if (!ev->in_event_class ("rhythmic-event"))
    {
      info.grob ()->warning (_ ("stem must have Rhythmic structure"));
      return;
    }

  last_stem_added_at_ = now;
  int durlog = unsmob_duration (ev->get_property ("duration"))->duration_log ();
  if (durlog <= 2)
    {
      ev->origin ()->warning (_ ("stem doesn't fit in beam"));
      prev_start_ev_->origin ()->warning (_ ("beam was started here"));
      /*
	don't return, since

	[r4 c8] can just as well be modern notation.
      */
    }

  stem->set_property ("duration-log",
		      scm_from_int (durlog));
  Moment stem_location = now - beam_start_mom_ + beam_start_location_;
  beam_info_->add_stem (stem_location,
			max (durlog- 2, 0));
  Beam::add_stem (beam_, stem);
}

ADD_ACKNOWLEDGER (Beam_engraver, stem);
ADD_ACKNOWLEDGER (Beam_engraver, rest);

ADD_TRANSLATOR (Beam_engraver,
		/* doc */

		"Handles Beam events by engraving Beams.  If omitted, then notes will be "
		"printed with flags instead of beams.",
		
		/* create */ "Beam",

		/* read */
		"beamMelismaBusy "
		"beatLength "
		"subdivideBeams "
		,
		/* write */
		"forbidBreak");

class Grace_beam_engraver : public Beam_engraver
{
public:
  TRANSLATOR_DECLARATIONS (Grace_beam_engraver);

  DECLARE_TRANSLATOR_LISTENER (beam);
  
protected:
  virtual bool valid_start_point ();
  virtual bool valid_end_point ();
};

Grace_beam_engraver::Grace_beam_engraver ()
{
}

bool
Grace_beam_engraver::valid_start_point ()
{
  Moment n = now_mom ();

  return n.grace_part_ != Rational (0);
}

bool
Grace_beam_engraver::valid_end_point ()
{
  return beam_ && valid_start_point ();
}

/*
  Ugh, C&P code.
 */
IMPLEMENT_TRANSLATOR_LISTENER (Grace_beam_engraver, beam);
void
Grace_beam_engraver::listen_beam (Stream_event *ev)
{
  Direction d = to_dir (ev->get_property ("span-direction"));

  if (d == START && valid_start_point ())
    start_ev_ = ev;
  else if (d == STOP && valid_end_point ())
    now_stop_ev_ = ev;
}


ADD_ACKNOWLEDGER (Grace_beam_engraver, stem);
ADD_ACKNOWLEDGER (Grace_beam_engraver, rest);

ADD_TRANSLATOR (Grace_beam_engraver,

		/* doc */

		"Handles Beam events by engraving Beams.  If omitted, then notes will "
		"be printed with flags instead of beams. Only engraves beams when we "
		" are at grace points in time. ",
		
		/* create */ "Beam",
		/* read */
		"beamMelismaBusy "
		"beatLength "
		"allowBeamBreak "
		"subdivideBeams "
		,
		/* write */ "");

