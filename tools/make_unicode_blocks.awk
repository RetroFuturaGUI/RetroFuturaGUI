# Makes source/Fonts/UnicodeBlocks.hpp from Unicode's Blocks.txt, keeping the names the current header gives its blocks.
#
# For a new Unicode version, download https://www.unicode.org/Public/<version>/ucd/Blocks.txt and run, from RetroFuturaGUI:
#     awk -f tools/make_unicode_blocks.awk source/Fonts/UnicodeBlocks.hpp Blocks.txt > UnicodeBlocks.new.hpp
# then replace the header with UnicodeBlocks.new.hpp. Needs GNU awk (strtonum), which is the awk Git Bash comes with.
#
# A block keeps its old name when its old range starts inside it; a new block takes Unicode's name without spaces and
# hyphens. It stops with an error when a new name is taken or an old block matches no block or more than one.

function hex(value) { return sprintf("0x%XU", value) }

# pass 1: the current header's names and bounds
FNR == NR {
    if ($1 == "#define" && match($2, /(First|Last)$/)) {
        name = substr($2, 1, RSTART - 1)
        value = strtonum(substr($3, 1, length($3) - 1))
        if (substr($2, RSTART) == "First") { oldFirst[name] = value; oldNames[++oldCount] = name }
        else oldLast[name] = value
    }
    next
}

# pass 2: Blocks.txt, whose first line names the version, like "# Blocks-18.0.0.txt"
/^# Blocks-[0-9]/ {
    match($0, /[0-9]+\.[0-9]+/)
    version = substr($0, RSTART, RLENGTH)
    next
}

/^[0-9A-F]+\.\.[0-9A-F]+; / {
    split($0, parts, /; /)
    split(parts[1], bounds, /\.\./)
    first = strtonum("0x" bounds[1])
    last = strtonum("0x" bounds[2])
    title = parts[2]
    sub(/\r$/, "", title)

    # Basic Latin is split, and its control characters stay out of the array
    if ("Basic Latin" == title) {
        add("ControlBlock", 0x0, 0x1F, 0)
        add("BasicLatin", 0x20, 0x7F, 1)
        used["ControlBlock"] = used["BasicLatin"] = 1
        next
    }

    name = ""
    for (i = 1; i <= oldCount; ++i) {
        old = oldNames[i]
        if (old == "ControlBlock" || old == "BasicLatin")
            continue
        if (first <= oldFirst[old] && oldFirst[old] <= last) {
            if (name != "") { print "ERROR: " title " matches " name " and " old > "/dev/stderr"; exit 1 }
            name = old
        }
    }

    if (name != "") {
        used[name] = 1
        kept++
    } else {
        name = title
        gsub(/[ -]/, "", name)
        if (name in oldFirst) { print "ERROR: new name " name " is taken" > "/dev/stderr"; exit 1 }
        added++
    }

    add(name, first, last, 1)
}

function add(name, first, last, inArray) {
    names[++count] = name
    firsts[count] = first
    lasts[count] = last
    arrayed[count] = inArray
    if (inArray) arrayCount++
}

END {
    if (version == "") { print "ERROR: the second file is no Blocks.txt, its first line names no version" > "/dev/stderr"; exit 1 }

    for (i = 1; i <= oldCount; ++i)
        if (!(oldNames[i] in used)) { print "ERROR: old block " oldNames[i] " has no Unicode block" > "/dev/stderr"; exit 1 }

    print "/* First/last codepoint bounds for each Unicode block, used to request glyph"
    print "    ranges from FontManager without hardcoding magic numbers at call sites."
    print "    Made by tools/make_unicode_blocks.awk from Unicode " version "'s Blocks.txt: for a new Unicode version, run it again"
    print "    rather than editing this by hand. The bounds are the whole block, not only its assigned characters, so characters"
    print "    added to a block later are covered too. Basic Latin is split in two: its control characters are ControlBlock, which"
    print "    stays out of UnicodeBlocks because FontManager::blockKeyOf returns 0 for \"no block\"."
    print "*/"
    print "#pragma once"
    print "#include \"IncludeHelper.hpp\""
    print "#include <array>"
    print ""

    for (i = 1; i <= count; ++i) {
        print "#define " names[i] "First " hex(firsts[i])
        print "#define " names[i] "Last " hex(lasts[i])
    }

    print ""
    print "static constexpr std::array<std::pair<u32, u32>, " arrayCount "> UnicodeBlocks = "
    print "{"
    written = 0
    for (i = 1; i <= count; ++i) {
        if (!arrayed[i]) continue
        written++
        print "    std::pair{ " names[i] "First, " names[i] "Last }" (written < arrayCount ? "," : "")
    }
    print "};"

    printf "Unicode %s: %d blocks (%d in the array), %d names kept, %d new\n", version, count, arrayCount, kept + 2, added > "/dev/stderr"
}
