# Generated loaders and REL integration

Loader bodies remain generator-owned. Change `generate_script_loaders.py` or the
template inputs, not individual `LoadTypedef` functions. Runtime tweak adapters
and REL setup/dispatch are separate, handwritten code.

## Tweaks profile

`config/loader_profiles/Tweaks.json` is the shared list of reviewed loader sources
for the Tweaks REL. `configure.py` reads this list; the generator uses it to select
outputs. Matching status still belongs to `configure.py`, not the generator.
The profile pins a template commit so remote generation does not follow `main`.

Generate into a staging directory for review:

```sh
uv run scripts/generate_script_loaders.py \
  --profile config/loader_profiles/Tweaks.json \
  --cpp-output build/tweaks-generated/src \
  --header-output build/tweaks-generated/include
```

Compare checked-in sources and headers without writing:

```sh
uv run scripts/generate_script_loaders.py \
  --profile config/loader_profiles/Tweaks.json \
  --cpp-output src/MetroidPrime/ScriptLoader \
  --header-output include/MetroidPrime/ScriptLoader --check
```

Both commands accept `--templates /path/to/retro-script-object-templates` for
offline input. Local input uses the actual files, including local modifications;
the profile does not assert that a local checkout matches its pinned commit.
Use the pinned revision when reproducing a published result.

`--check` exits 1 for missing or differing files and 0 for identical outputs,
ignoring CRLF versus LF. It never creates directories or overwrites files.
Ordinary generation refuses differing existing files unless `--force` is given.
Review staged output before intentionally regenerating tracked files.

Profiles select outputs after resolving the full game type graph. This is
important: a helper shared with a non-Tweaks object must keep its shared header,
not move into a tweak header just because that object was omitted from a run.
Sources include their owning headers; exclusively used helpers remain in their
parent's header. Header output includes transitive generated-header dependencies,
but source output includes only the explicitly reviewed units.

## Adding units

1. Establish native function identity and boundaries in the REL's symbols/splits.
   Template filenames and adjacent triples alone do not prove an original TU.
2. Add the source path relative to `MetroidPrime/ScriptLoader` to the profile.
3. Generate and review the output; make any necessary corrections in the generator.
4. Compile with the game compiler/flags. RELs add `-sdata 0 -sdata2 0` because
   relocatable code cannot use the main DOL's small-data bases.
5. Verify instructions, call targets, data ownership and final source-linked hashes
   before changing matching status. Exact method bodies are not sufficient.

The current profile covers the 16 already integrated loader units. It deliberately
does not register every XML type: the remaining nested readers still need native
split/ownership review. Tweaks constructors share native floating constants across
many loader groups; separate logical splits must not be mistaken for independent
original compilation units or independently reproducible constant pools.

Generation is explicit, not an implicit build-time network operation. Normal
builds use checked-in outputs and need no template checkout. CI checks that the
reviewed files can be reproduced from the pinned templates.

## Generator tests

```sh
python -m unittest discover -s scripts/tests -p test_generate_script_loaders.py
uvx ruff check scripts/generate_script_loaders.py tools/loader_profile.py scripts/tests/test_generate_script_loaders.py
uvx ruff format --check scripts/generate_script_loaders.py tools/loader_profile.py scripts/tests/test_generate_script_loaders.py
```

The tests use in-memory XML and need neither original game files nor network access.
