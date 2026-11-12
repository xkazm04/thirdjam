"""tools/p4/alias_of.py <vendor_lower> [bone]: the bone alias map of a vendor config (JSON), or one aliased bone name."""
import json, sys, os
p = os.path.join(os.path.dirname(os.path.abspath(__file__)), "jobs", sys.argv[1] + ".ue.json")
a = json.load(open(p)).get("alias", {}) if os.path.exists(p) else {}
print(a.get(sys.argv[2], sys.argv[2]) if len(sys.argv) > 2 else json.dumps(a).replace('"', "'"))   # a Python literal with no double quotes: it is pasted into MCP arg JSON
