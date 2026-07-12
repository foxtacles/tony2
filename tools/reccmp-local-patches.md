# Local reccmp patches

The venv's reccmp (installed from git+https://github.com/isledecomp/reccmp) carries
one local patch. Reapply after any `pip install -r tools/requirements.txt`.

## Symbolize displacements in indirect calls (compare/asm/parse.py)

`sanitize()` skips the `displace_replace_regex` pass for `call` instructions, so an
indirect call through a global function-pointer table
(`call dword ptr [eax + 0x4ef724]`) keeps a raw, binary-specific displacement on
both sides and can never match. Patch: after the absolute-indirect substitution in
the `call` branch, also run

    op_str = displace_replace_regex.sub(self.hex_replace_relocated, op_str)

Small vtable/member displacements are unaffected (they fail the address test).
Un-parks FUN_004267c6 (table dispatch at g_unk0x4ef720[i].m_unk0x04). Candidate for
upstreaming.
