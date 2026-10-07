// Config.h
// ---------------------------------------------------------------------------
// Central place for every "judgement call" the project specification leaves
// open.  Flip a flag here and rebuild if your instructor wants it differently.
// ---------------------------------------------------------------------------
#pragma once

namespace config {

// true  -> "If", "IF", "if" are all the reserved word 'if' (spec examples mix
//          cases, e.g. "If(x<5) Output<<X;").  Identifiers stay case-sensitive.
inline constexpr bool CASE_INSENSITIVE_KEYWORDS = true;

// true  -> both  x:=5;  and  x=5;  are accepted as assignment (the spec uses
//          both forms).  false -> only ':=' is accepted.
inline constexpr bool ALLOW_PLAIN_EQUALS = true;

// true  -> integer literals must be a single digit (0-9) and double literals
//          may have at most 2 decimal places, as the spec states.
inline constexpr bool ENFORCE_LITERAL_LIMITS = true;

// true  -> line breaks are kept in NOSPACES.TXT (only blanks/tabs removed).
// false -> the whole program is written as one long line.
inline constexpr bool KEEP_NEWLINES_IN_NOSPACES = true;

// true  -> blanks inside "string literals" are removed too (spec: "removes all
//          spaces in the program").  This only affects NOSPACES.TXT; the
//          interpreter always uses the original text so output is correct.
inline constexpr bool STRIP_SPACES_IN_STRINGS = true;

inline constexpr const char* NOSPACES_FILE = "NOSPACES.TXT";
inline constexpr const char* RES_SYM_FILE  = "RES_SYM.TXT";

}  // namespace config
