# Contributing

## Important Note

While we're thrilled that there is interest in reverse engineering Tony & Friends: New Adventures
and are happy to accept contributions from anyone who would like to help, proposed changes to this
repository must adhere to a certain degree of engineering quality. While the established
contributors here are more than happy to provide code reviews and constructive criticism, it is not
their job to teach potential contributors C++ or decompilation fundamentals. As a project that is
largely an artifact of the free time of its contributors, the more of that (often scarce) resource
that can be dedicated to efficient work, the faster the project will progress. Unfortunately,
well-intentioned but poorly constructed contributions can actually hurt progress in the long term.
While we are greatly appreciative of the sentiment, if you aren't very confident in your
decompilation abilities, it is generally in the project's best interest that you return when you
have a better grasp over the process.

Generally, decompilation is a fairly advanced skill. Depending on your current proficiency with
C/C++ and x86 assembly, it could take you months or even years to learn the skills necessary to do
it adequately.

## General Guidelines

If you feel fit to contribute, feel free to create a pull request! Someone will review and merge it
(or provide feedback) as soon as possible.

Please keep your pull requests small and understandable; you may be able to shoot ahead and make a
lot of progress in a short amount of time, but this is a collaborative project, so you must allow
others to catch up and follow along. Large pull requests become significantly more unwieldy to
review, and as such make it exponentially more likely for a mistake or error to go undetected. They
also make it harder to merge other pull requests because the more files you modify, the more likely
it is for a merge conflict to occur. A general guideline is to keep submissions limited to one class
at a time. Sometimes two or more classes may be too interlinked for this to be feasible, so this is
not a hard rule, however if your PR is starting to modify more than 10 or so files, it's probably
getting too big.

This repository currently has only one goal: accuracy to the original executable. We are
byte/instruction matching as much as possible, which means the priority is making the original
compiler (MSVC 5.0 SP3, plus MSVC 6.0 RTM for the sound library) produce code that matches the
original game. As such, modernizations and bug fixes will probably be rejected for the time being.

## Overview

* [`TONY2`](/TONY2): Decompilation of `TONY2.EXE`.
* [`util`](/util): Utility headers aiding in the decompilation effort.

## Code Style

In general, we're not exhaustively strict about coding style, but there are some preferable
guidelines to follow that have been adopted from what we know about the original codebase.

### Formatting

We use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and
[clang-tidy](https://clang.llvm.org/extra/clang-tidy/) with configuration files that aim to
replicate the code formatting employed by the original developers. There are
[integrations](https://clang.llvm.org/docs/ClangFormat.html#vim-integration) available for most
editors and IDEs.

### Naming conventions

We use the same naming conventions as the [LEGO Island decompilation](https://github.com/isledecomp/isle):
- Unknown functions: `FUN_XXXXXXXX` (8 hex digits, lowercase)
- Unknown global variables: `g_unk0xXXXXXXXX`
- Unknown member variables: `m_unk0xXX`
- Unknown parameters: `p_unk0xXX`

See `CLAUDE.md` for the full set of TONY2-specific conventions and codegen notes.
