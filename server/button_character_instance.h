// Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// SWF buttons.  Mouse-sensitive update/display, actions, etc.

/* $Id: button_character_instance.h,v 1.13 2007/02/28 17:25:25 udog Exp $ */

#ifndef GNASH_BUTTON_CHARACTER_INSTANCE_H
#define GNASH_BUTTON_CHARACTER_INSTANCE_H


#include "character.h" // for inheritance


namespace gnash {

// Forward declarations
	class sprite_instance;
	class button_character_definition;

//
// button characters
//

enum mouse_state
{
	MOUSE_UP,
	MOUSE_DOWN,
	MOUSE_OVER
};

//
// button_character_instance
//

class button_character_instance : public character
{
public:
	button_character_definition*	m_def;
	std::vector< boost::intrusive_ptr<character> >	m_record_character;

	enum mouse_flags
	{
		IDLE = 0,
		FLAG_OVER = 1,
		FLAG_DOWN = 2,
		OVER_DOWN = FLAG_OVER|FLAG_DOWN,

		// aliases
		OVER_UP = FLAG_OVER,
		OUT_DOWN = FLAG_DOWN
	};
	int	m_last_mouse_flags, m_mouse_flags;
	enum e_mouse_state
	{
		UP = 0,
		DOWN,
		OVER
	};
	e_mouse_state m_mouse_state;

	button_character_instance(button_character_definition* def,
			character* parent, int id);

	~button_character_instance();

	bool can_handle_mouse_event() const { return true; }

	// called from keypress listener only
	bool on_event(const event_id& id);

	void	restart();

	virtual void	advance(float delta_time);

	void	display();

	/// Combine the flags to avoid a conditional.
	//  It would be faster with a macro.
	inline int	transition(int a, int b) const
	{
		return (a << 2) | b;
	}


	/// \brief
	/// Return the topmost entity that the given point covers. 
	/// NULL if none.
	//
	/// I.e. check against ourself.
	///
	virtual character* get_topmost_mouse_entity(float x, float y);

	virtual void	on_button_event(const event_id& event);

	void restart_characters(int condition);

	//
	// ActionScript overrides
	//

	void add_invalidated_bounds(InvalidatedRanges& ranges, bool force);
	

	// not sure if we need to override this one.
	//virtual const char*	get_text_value() const { return NULL; }	// edit_text_character overrides this
	
	virtual float get_width() const;

	virtual float get_height() const;

};

}	// end namespace gnash


#endif // GNASH_BUTTON_CHARACTER_INSTANCE_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
