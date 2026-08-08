"""
Dump alchemy data assets into the Obsidian design vault.

Usage — from the Unreal editor, Output Log window, console dropdown set to "Python":

    import dump_alchemy_vault
    dump_alchemy_vault.run()

To reload after editing this file:

    import importlib, dump_alchemy_vault
    importlib.reload(dump_alchemy_vault)
    dump_alchemy_vault.run()

WHAT IT TOUCHES
    Only the YAML frontmatter block of each note. Everything below the closing
    "---" is left byte-for-byte alone, so hand-written design prose survives a
    regeneration. Notes that do not exist yet are created with a stub body.

    A note is matched to an asset by its `asset:` frontmatter key, NOT by
    filename — so you can rename or move a note freely.
"""

import json
import os
import unreal


VAULT = r"C:\Users\marti\Obsidian\AlchemyGame"

# UE class name -> (vault subfolder, ordered frontmatter keys to manage)
TYPES = {
    "DataAssetSubstanceDefinition": (
        "02 Substances",
        ["DisplayName", "SubstanceTag", "BaseToxicity", "HeatStability",
         "DryingStability", "CrushingStability", "ColorHint", "Properties",
         "BaseEffects"],
    ),
    "DataAssetAlchemyEfectDefinition": (  # spelled this way in-repo
        "03 Effects",
        ["DisplayName", "EffectTag", "bIsPositive", "bIsRootCauseTreatment",
         "bIsSymptomRelief", "bIsSideEffect"],
    ),
    "DataAssetProcessingMethod": (
        "06 Processing",
        ["DisplayName", "ProcessingTag", "GeneralPotencyMultiplier",
         "ToxicityMultiplier", "StabilityMultiplier"],
    ),
    "DataAssetDisease": (
        "05 Diseases",
        ["DisplayName", "DiseaseTags", "Symptoms", "RelevantNutrients",
         "MaxAllowedToxicity", "ProgressionSpeed", "TreatmentRequirements"],
    ),
    "DataAssetPlantPart": (
        "04 Herbs",
        ["DisplayName", "PlantPartTag", "BaseQuality", "bCanBeProcessed",
         "AllowedProcessingTags", "Substances"],
    ),
}

def _snake(name):
    out = ""
    for i, ch in enumerate(name):
        if ch.isupper() and i > 0 and not name[i - 1].isupper():
            out += "_"
        out += ch.lower()
    return out


def _candidates(cpp_name):
    """C++ property name -> candidate Python names. UE strips the leading "b"
    from bools, and the exact form has shifted between engine versions, so try
    the most likely spellings in order."""
    out = []
    if cpp_name.startswith("b") and len(cpp_name) > 1 and cpp_name[1].isupper():
        out.append(_snake(cpp_name[1:]))   # bIsPositive -> is_positive
    out.append(_snake(cpp_name))           # BaseToxicity -> base_toxicity
    out.append(cpp_name)                   # last resort: verbatim
    return out


def get_prop(obj, cpp_name):
    """Read a UPROPERTY, tolerating engine-version naming differences."""
    for name in _candidates(cpp_name):
        try:
            return obj.get_editor_property(name)
        except Exception:
            continue
    return None


# ---------- value formatting ----------

def fmt_tag(tag):
    if tag is None:
        return ""
    try:
        name = str(tag.get_editor_property("tag_name"))
    except Exception:
        name = str(tag)
    return "" if name in ("None", "") else name


def fmt_tag_container(container):
    if container is None:
        return []
    for attr in ("gameplay_tags", "tags"):
        try:
            tags = container.get_editor_property(attr)
            return [t for t in (fmt_tag(x) for x in tags) if t]
        except Exception:
            continue
    text = str(container)
    return [text] if text and text != "None" else []


def fmt_color(color):
    if color is None:
        return ""
    try:
        to255 = lambda v: max(0, min(255, int(round(float(v) * 255))))
        return "#{:02X}{:02X}{:02X}".format(
            to255(color.r), to255(color.g), to255(color.b)
        )
    except Exception:
        return str(color)


def asset_name(obj):
    if obj is None:
        return ""
    try:
        return obj.get_name()
    except Exception:
        return str(obj)


def fmt_num(value):
    if value is None:
        return ""
    try:
        f = float(value)
    except (TypeError, ValueError):
        return str(value)
    return str(int(f)) if f == int(f) else "{:g}".format(f)


def fmt_scalar(value):
    """Format a single frontmatter value as YAML."""
    if value is None:
        return '""'
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float)):
        return fmt_num(value)
    if isinstance(value, list):
        return "[" + ", ".join(fmt_scalar(v) for v in value) + "]"
    text = str(value)
    if text == "":
        return '""'
    if any(c in text for c in ':#"\'[]{},&*?|<>=!%@`') or text.strip() != text:
        return '"' + text.replace('\\', '\\\\').replace('"', '\\"') + '"'
    return text


# ---------- per-type extraction ----------

def extract(obj, cls_name, keys):
    """Return {frontmatter_key: value} plus a list of markdown detail lines."""
    data = {}
    detail = []

    for key in keys:
        raw = get_prop(obj, key)

        if key in ("BaseEffects", "Substances", "TreatmentRequirements"):
            continue  # arrays of structs -> rendered as tables, not frontmatter

        if key.endswith("Tag"):
            data[key] = fmt_tag(raw)
        elif key.endswith("Tags") or key in ("Properties", "Symptoms", "RelevantNutrients"):
            data[key] = fmt_tag_container(raw)
        elif key == "ColorHint":
            data[key] = fmt_color(raw)
        elif key == "DisplayName":
            data[key] = str(raw) if raw is not None else ""
        elif isinstance(raw, bool):
            data[key] = raw
        elif isinstance(raw, (int, float)):
            data[key] = raw
        else:
            data[key] = "" if raw is None else str(raw)

    if "BaseEffects" in keys:
        rows = []
        for entry in (get_prop(obj, "BaseEffects") or []):
            effect = get_prop(entry, "Effect")
            rows.append("| [[{}]] | {} |".format(
                pretty(asset_name(effect)), fmt_num(get_prop(entry, "Value"))
            ))
        detail += table("Base Effects", ["Effect", "Value"], rows)

    if "Substances" in keys:
        rows = []
        for entry in (get_prop(obj, "Substances") or []):
            substance = get_prop(entry, "Substance")
            rows.append("| [[{}]] | {} |".format(
                pretty(asset_name(substance)), fmt_num(get_prop(entry, "Amount"))
            ))
        detail += table("Substances", ["Substance", "Amount"], rows)

    if "TreatmentRequirements" in keys:
        rows = []
        for entry in (get_prop(obj, "TreatmentRequirements") or []):
            effect = get_prop(entry, "RequiredEffect")
            mandatory = get_prop(entry, "bIsMandatory")
            rows.append("| [[{}]] | {} | {} |".format(
                pretty(asset_name(effect)),
                fmt_num(get_prop(entry, "RequiredValue")),
                "true" if mandatory else "false",
            ))
        detail += table("Treatment Requirements",
                        ["Effect", "RequiredValue", "Mandatory"], rows)

    return data, detail


def table(title, headers, rows):
    out = ["## {} (generated)".format(title), ""]
    if not rows:
        out += ["_none authored_", ""]
        return out
    out.append("| " + " | ".join(headers) + " |")
    out.append("|" + "|".join(["---"] * len(headers)) + "|")
    out += rows
    out.append("")
    return out


def pretty(asset):
    """Asset name -> vault note title.

    DA_Substance_Frostaline    -> Frostaline
    DA_Herb_Mint_Leaf          -> Mint Leaf
    DA_Effect_FightInfection   -> Fight Infection

    The CamelCase split matters: these strings become [[wikilinks]], so a name
    that doesn't match the note title produces a dead link in the graph.
    """
    name = asset
    for prefix in ("DA_Substance_", "DA_Effect_", "DA_Processing_",
                   "DA_Disease_", "DA_Herb_", "DA_Tool_", "DA_Tools_", "DA_"):
        if name.startswith(prefix):
            name = name[len(prefix):]
            break
    name = name.replace("_", " ")
    out = ""
    for i, ch in enumerate(name):
        if (ch.isupper() and i > 0
                and (name[i - 1].islower() or name[i - 1].isdigit())):
            out += " "
        out += ch
    return " ".join(out.split())


# ---------- note read / write ----------

MARK_BEGIN = "<!-- generated:begin -->"
MARK_END = "<!-- generated:end -->"


def split_note(text):
    """-> (frontmatter_lines, body). Frontmatter excludes the --- fences."""
    if not text.startswith("---"):
        return [], text
    lines = text.splitlines()
    for i in range(1, len(lines)):
        if lines[i].strip() == "---":
            return lines[1:i], "\n".join(lines[i + 1:])
    return [], text


def merge_frontmatter(existing, updates):
    """Update managed keys in place; append any that are missing. Key order and
    unmanaged keys (type, status, notes of your own) are preserved."""
    seen = set()
    out = []
    for line in existing:
        key = line.split(":", 1)[0].strip() if ":" in line else None
        if key in updates:
            out.append("{}: {}".format(key, fmt_scalar(updates[key])))
            seen.add(key)
        else:
            out.append(line)
    for key, value in updates.items():
        if key not in seen:
            out.append("{}: {}".format(key, fmt_scalar(value)))
    return out


def replace_generated(body, detail_lines):
    """Swap the marked generated block, or append one if absent."""
    block = "\n".join([MARK_BEGIN, ""] + detail_lines + [MARK_END])
    if MARK_BEGIN in body and MARK_END in body:
        head = body.split(MARK_BEGIN, 1)[0]
        tail = body.split(MARK_END, 1)[1]
        return head + block + tail
    return body.rstrip() + "\n\n" + block + "\n"


def index_notes():
    """Map asset name -> note path, by reading each note's `asset:` key."""
    found = {}
    for root, dirs, files in os.walk(VAULT):
        dirs[:] = [d for d in dirs if d != ".obsidian"]
        for filename in files:
            # "_Substance Template.md" etc. carry a bare `asset: DA_Substance_`
            # prefix as a fill-in hint. They must never be treated as a note for
            # a real asset, or a regeneration would overwrite the template.
            if not filename.endswith(".md") or filename.startswith("_"):
                continue
            path = os.path.join(root, filename)
            try:
                with open(path, "r", encoding="utf-8") as handle:
                    front, _ = split_note(handle.read())
            except (IOError, OSError):
                continue
            for line in front:
                if line.startswith("asset:"):
                    value = line.split(":", 1)[1].strip().strip('"')
                    # a bare prefix like "DA_Substance_" is an unfilled stub
                    if value and not value.endswith("_"):
                        found[value] = path
                    break
    return found


def stub(name, cls_name):
    return "\n".join([
        "# " + pretty(name),
        "",
        "_Design notes go here — the frontmatter and generated block below are",
        "overwritten by `dump_alchemy_vault.py`, but this prose is never touched._",
        "",
    ])


# ---------- main ----------

def run(dry_run=False):
    if not os.path.isdir(VAULT):
        unreal.log_error("[vault] Vault not found: {}".format(VAULT))
        return

    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    all_assets = registry.get_assets_by_path("/Game", recursive=True)
    notes = index_notes()

    written, created, skipped = 0, 0, 0

    for asset_data in all_assets:
        try:
            cls_name = str(asset_data.asset_class_path.asset_name)
        except AttributeError:
            cls_name = str(asset_data.asset_class)

        if cls_name not in TYPES:
            continue

        folder, keys = TYPES[cls_name]
        obj = asset_data.get_asset()
        if obj is None:
            skipped += 1
            continue

        name = asset_name(obj)
        try:
            data, detail = extract(obj, cls_name, keys)
        except Exception as exc:
            unreal.log_error("[vault] {} failed: {}".format(name, exc))
            skipped += 1
            continue

        data["asset"] = name
        # every value above came straight from the engine this run, so any
        # "needs-transcription" marker left on the note is now a lie
        data["status"] = "synced"

        path = notes.get(name)
        if path is None:
            path = os.path.join(VAULT, folder, pretty(name) + ".md")
            front, body = ["type: " + folder.split(" ", 1)[1].lower().rstrip("s")], \
                          stub(name, cls_name)
            created += 1
        else:
            with open(path, "r", encoding="utf-8") as handle:
                front, body = split_note(handle.read())
            written += 1

        front = merge_frontmatter(front, data)
        body = replace_generated(body, detail)
        text = "---\n" + "\n".join(front) + "\n---\n" + body

        if dry_run:
            unreal.log("[vault] would write {}".format(path))
            continue

        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)

    if not dry_run:
        # Tools/vault_check.py compares this against .uasset mtimes to tell you
        # when the vault has fallen behind the editor.
        import time
        state = os.path.join(VAULT, ".sync-state.json")
        try:
            with open(state, "w", encoding="utf-8") as handle:
                json.dump({
                    "last_sync_epoch": time.time(),
                    "last_sync_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ",
                                                   time.gmtime()),
                    "updated": written,
                    "created": created,
                    "skipped": skipped,
                }, handle, indent=2)
        except (IOError, OSError) as exc:
            unreal.log_error("[vault] could not write sync state: {}".format(exc))

    unreal.log("[vault] updated {}, created {}, skipped {}".format(
        written, created, skipped))


if __name__ == "__main__":
    run()
