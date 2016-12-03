#include <utility>
#include "AudioSignal.h"

/**
   \brief A simple note generator that converts standard Western Music
   notes "CDEFGAB" at different Octaves (0 through 6) and Tempos (32
   through 255).

   <p>This method converts a valid note string representations to
   corresponding numerical note values in the range 0-84 based on the
   current octave.  Valid note string representations are in the form
   <tt>A-G[#,+,-][Length][.]</tt> where \c A through \c G are notes. A
   \c # or \c + character following a note produces a sharp or the
   next higher note while \c - after a note produces a flat. Examples
   of valid notes includes: A B E3 C# C#2..</p>

   <p>If an optional numeric length value is specified after the note,
   then the specified length value is used instead of the default note
   length set for the score.</p>
   
   <p>A period (\c .) after a note increases the playing time of the
   note by 3/2 times the period determined by length of note times the
   tempo.  Multiple periods can appear after a note, and the playing
   time is scaled accordingly. For example, \c A. will cause the note
   \c A to play one and half times the playing time determined by
   length of the note times the tempo.  Similarly, two periods placed
   after A (\c A..)  will cause the note to be played at 9/4 times its
   ascribed value; an \c A with three periods (\c A...) at 27/8, and
   so on.</p>

   In addition to the notes this class also handles the following
   meta notes:

   <ul>

   <li> <b>L#</b> (where \c # is an integer in the range 1 through 32):
   Changes the default length of notes. Example: \c L1 (full length
   notes), \c L2 (1/2 length notes), \c L4 (1/4 length notes)
   etc.</li>

   <li> <b>O#</b> (where \c # is an integer in the range 1 through 6):
   Changes the default octave (or frequency) for the notes.  Example:
   \c O1 (low frequency), \c O6 (high frequency).</li>

   <li> <b>T#</b> (where \c # is an integer in the range 32 through
   254): Changes the default tempo (or duration) for the notes.
   Example: \c T64 (notes are long), \c T200 (short notes).</li>

   <li> <b>T#</b> (where \c # is an integer in the range 1 through
   32): Introduces a pause (or silent note, with amplitude zero) for
   the given note length.</li>

   <li> <b>V#</b> (where \c # is an integer in the range 1 through
   20000): Changes the volume (or peak amplitude) of a given
   note.</li>

   <li> <b> \< </b>: Decreases the current octave by one.</li>

   <li> <b> \< </b>: Increases the current octave by one.</li>   

   </ul>

   Here is the music notes associated with the nursery rhyme "mary had
   a little lamb":

   \code

   b a g a b b b2
   a a a2 b b b2 b a g a b b b b
   a a b a g1

   \endcode
*/
class NoteMaker {
private:
    /**
       Instance variable to track the current tempo value used to
       generate audio signals.  The tempo determines the number of
       quarter notes to be generated per minute.  For example, if the
       tempo is 60 then 60 quarter notes (or 15 full notes) are
       generated per minute which means that each quarter note is 1
       second long.  The default tempo value is 120 (making each
       quarter note 0.5 seconds long).  Valid values for tempo are 32
       through 255.
    */
    int tempo;
    
    /** The default duration / length a note occupies given the
        current tempo.
        
        The length value determines the actual time-duration of a note
        at a given tempo.  The length value can be in the range 1 to
        64, with 4 as the default value.  Given length of 1 to 64
        (that corresponds to various note lengths) the actual time (in
        seconds) for a music note can be determined using the formula:

        noteDuration (in seconds) = 60.0 * 4.0 / length / tempo
    */
    int noteLength;

    /** The default Octave in which the notes are to be generated.

        The Octave essentially determines the frequency or pitch of
        the music notes generated. Valid Octave values are 0 to 6.
        The default Octave value is 4.  A zero octave value
        corresponds to zero frequency (or pure silence). Higher octave
        values indicate higher frequency or high pitch.  Each octave
        consists of the following 12 notes: {C, C#, D, D#, E, F, F#,
        G, G#, A, A#, B}.  In a given octave, each of these 12 notes
        are assigned consecutive numbers in the range 0 to 11 (similar
        to index values in an array).  Furthermore, given 1 through 6
        octaves (each octave having 12 notes), the total number of
        notes this class must handle is 7 * 12 = 84 notes.  The
        frequencies corresponding to each of the 84 notes (or keys on
        a piano) can be computed using the formula:

        freq = 440 * exp(log(2) * (note - 46) / 12)

        An very nice article explaining the above formula with mapping
        to piano keyboards is available at: http://en.wikipedia.org/wiki/Piano_key_frequencies
    */
    int octave;

	/**
	   The peak amplitude or peak volume for the audio notes generated
	   by this note maker.  This value is essentially the amplitude
	   parameter passed to the AudioSignal class.  The default value
	   is 15000.
	*/
	int volume;
	
public:
    /**
       Constructor to create a note maker.

       The constructor permits creating a note maker with suitable
       parameters.

       \param[in] tempo The tempo determines the number of quarter
       notes to be generated per minute.  For example, if the tempo is
       60 then 60 quarter notes (or 15 full notes) are generated per
       minute which means that each quarter note is 1 second long.
       The default tempo value is 120 (making each quarter note 0.5
       seconds long).  Valid values for tempo are 32 through 255.

       \param[in] noteLength The length value determines the actual
       duration of a note at a given tempo.  The length value can be
       in the range 1 to 64, with 4 as the default value.

       \param[in] octave The initial octave that determines the
       frequency or pitch of the music notes generated by this class.

	   \param[in] volume The peak amplitude or peak volume for the
	   audio notes generated by this note maker.
    */
    NoteMaker(int tempo = 120, int noteLength = 4, int octave = 4,
			  int volume = 15000);
    
    /**
       Convenience method to convert a note (in range 1 to 84) to a
       fixed frequency value.

       \param[in] note A note in the range 0 to 84 to be converted to
       a predefined frequency value. See
       http://en.wikipedia.org/wiki/Piano_key_frequencies for details
       on the frequencies for various notes.  Each note value maps to
       one of the following 12 notes in 7 different octaves: {C, C#,
       D, D#, E, F, F#, G, G#, A, A#, B}.

       \return The frequency corresponding to the given note.
    */
    int note2Freq(const int note) const;

    /** Functor to convert a given note string to a corresponding
        audio signal.

        This is a convenience method that can be used to convert a
        valid note string to a given audio signal.  Refer to the
        documentation on this class for details on valid note strings.

        \param[in] entry The string representation of a musical note
        or various meta notes to be converted.  Music notes are
        converted to an audio signal based on the current octave,
        tempo, and noteLength.  Meta notes are suitably processed to
        update the internal parameters of this class.

        \return An audio signal that represents the given music meta
        note.  If the meta note is an meta command then this method
        returns a empty AudioSignal.  If the meta note is an actual
        music note, then this method returns a suitable audio signal
        of given frequency and noteLength.

        \exception std::exception This method throws an exception with
        the message (<tt>The following meta note is invalid:
        <i>metaNote</i></tt>) where <i>metaNote</i> is the parameter
        passed to this method.  This exception is thrown (with the
        metaNote to be coverted) if the note is invalid.  The result
        of calling this method on an invalid note is not specified.
    */
    AudioSignal operator()(const std::string& entry) throw
        (std::exception);

    /**
       A convenience method to convert a string to an integer.

       \param[in] str The string to be converted to a number.

       \return Returns the integer value represented by the number.

       \exception std::exception This method throws an exception if
       the string is not a valid integer.
    */
    static int toInt(const std::string& str) throw (std::exception);

    /**
       A convenience method to convert a string to a double.

       \param[in] str The string to be converted to a number.

       \return Returns the double value represented by the number.

       \exception std::exception This method throws an exception if
       the string is not a valid double.
    */
    static double toDouble(const std::string& str) throw (std::exception);
    
protected:
    /** Convert one of the string notes to a numerical note value in
        the range 0 to 84.

        <p>This method converts a valid note string representations to
        corresponding numerical note values in the range 0-84 based on
        the current octave.  Refer to the class documentation for
        details on valid note strings that this class can
        process.</p>

        \param[in] noteStr The string representation of a musical note
        to be converted to a numerical note.  

        \return This method returns a pair of values. The first value
        in the pair is the numerical note value in the range 0 to 84
        that can then be converted to suitably frequency value using
        the note2Freq method in this class.  The second value in the
        pair is the note duration in milliseconds that was computed by
        calling getNoteTime method.

        \exception std::exception This method throws an exception is
        the given noteStr is not valid.  Specifically, this method
        throws an std::runtime_error with the message (<tt>The
        following meta note is invalid: <i>metaNote</i></tt>) where
        <i>metaNote</i> is the parameter passed to this method.
    */
    std::pair<int, int> convert(const std::string& noteStr) const throw (std::exception);

    /**
       Meta notes 
     */
    int processMetaNote(const std::string& metaStr) throw (std::exception);
    
    /** Convert note length to time duration in milliseconds.

        This is a convenience method that can be used to convert note
        length to time in milliseconds.  This method uses the default
        duration value in the class that is modified by the number of
        dot values specified.

		\param[in] noteLen The length of the note that is to be used
		to compute the time duration for a note.
		
        \param[in] dots The number of periods by which the length of a
        note is to be increased.  For example, a dot value of \c 1
        will cause the note length play one and half times the playing
        time determined by length of the note times the tempo.
        Similarly, dot period of \c 2 will cause the note to be played
        at 9/4 times its ascribed value; a dot value of \c 3 will set
        the duration at at 27/8, and so on.
    */
    int getNoteTime(int noteLen, int dots = 0) const;

};

