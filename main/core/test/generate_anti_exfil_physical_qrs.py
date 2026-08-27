#!/usr/bin/env python3
"""Render exact stage-1/stage-3 measurement URs as animated QR packs."""

import argparse
import hashlib
import html
import json
import re
import shutil
import subprocess
from pathlib import Path


LINE = re.compile(r"^stage=(\d+) part=(\d+)/(\d+) (UR:.+)$")
ROOT = Path(__file__).resolve().parent


def emit(fragment: int):
    binary = ROOT / "measure_anti_exfil_transport"
    subprocess.run(["make", binary.name], cwd=ROOT, check=True)
    output = subprocess.run(
        [str(binary), "--emit", str(fragment)], cwd=ROOT, check=True,
        text=True, capture_output=True,
    ).stdout.splitlines()
    groups = {1: [], 3: []}
    for line in output:
        match = LINE.match(line)
        if not match:
            raise RuntimeError(f"unexpected emitter output: {line!r}")
        stage, current, total, ur = match.groups()
        stage, current, total = int(stage), int(current), int(total)
        if stage in groups:
            if current != len(groups[stage]) + 1:
                raise RuntimeError(f"stage {stage}: non-contiguous part sequence")
            groups[stage].append(ur)
            if current == total and len(groups[stage]) != total:
                raise RuntimeError(f"stage {stage}: inconsistent part total")
    if not all(groups.values()):
        raise RuntimeError("emitter did not produce both signer-side stages")
    return groups


def viewer(title: str, frames, default_ms: int):
    quoted = ",\n      ".join(json.dumps(frame) for frame in frames)
    return f"""<!doctype html>
<meta charset=\"utf-8\">
<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">
<title>{html.escape(title)}</title>
<style>
  html,body {{ margin:0; min-height:100%; background:#111; color:#eee;
    font:16px system-ui,sans-serif; text-align:center; }}
  main {{ display:flex; min-height:100vh; flex-direction:column; align-items:center; }}
  h1 {{ font-size:1.15rem; margin:.6rem 0 .2rem; }}
  #qr {{ width:min(78vh,92vw); height:min(78vh,92vw); object-fit:contain;
    image-rendering:pixelated; background:white; }}
  .controls {{ margin:.45rem; display:flex; gap:.45rem; align-items:center; flex-wrap:wrap;
    justify-content:center; }}
  button,input {{ font:inherit; padding:.35rem .6rem; }}
</style>
<main>
  <h1>{html.escape(title)} — <span id=\"part\"></span></h1>
  <img id=\"qr\" alt=\"Animated UR QR\">
  <div class=\"controls\">
    <button id=\"prev\">Previous</button><button id=\"toggle\">Pause</button>
    <button id=\"next\">Next</button><label>Frame ms
    <input id=\"delay\" type=\"number\" min=\"150\" step=\"50\" value=\"{default_ms}\"></label>
  </div>
</main>
<script>
  const frames = [
      {quoted}
  ];
  let index=0, playing=true, timer;
  const qr=document.querySelector('#qr'), part=document.querySelector('#part');
  function show(i) {{ index=(i+frames.length)%frames.length; qr.src=frames[index];
    part.textContent=`part ${{index+1}} / ${{frames.length}}`; }}
  function schedule() {{ clearTimeout(timer); if (playing) timer=setTimeout(() => {{
    show(index+1); schedule(); }}, Number(document.querySelector('#delay').value)); }}
  document.querySelector('#toggle').onclick=() => {{ playing=!playing;
    document.querySelector('#toggle').textContent=playing?'Pause':'Play'; schedule(); }};
  document.querySelector('#prev').onclick=() => {{ show(index-1); schedule(); }};
  document.querySelector('#next').onclick=() => {{ show(index+1); schedule(); }};
  document.querySelector('#delay').onchange=schedule;
  show(0); schedule();
</script>
"""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=ROOT / "physical_qr_output")
    parser.add_argument("--fragments", nargs="+", type=int, default=[150, 200])
    parser.add_argument("--frame-ms", type=int, default=700)
    args = parser.parse_args()
    qrencode = shutil.which("qrencode")
    if not qrencode:
        raise SystemExit("qrencode is required (on Ubuntu: sudo apt install qrencode)")
    args.output.mkdir(parents=True, exist_ok=True)
    manifest = {"generator": Path(__file__).name, "packs": []}
    links = []
    for fragment in args.fragments:
        for stage, parts in emit(fragment).items():
            pack = args.output / f"fragment-{fragment}" / f"stage-{stage}"
            pack.mkdir(parents=True, exist_ok=True)
            frame_names = []
            for index, ur in enumerate(parts, 1):
                name = f"part-{index:02d}-of-{len(parts):02d}.png"
                subprocess.run([
                    qrencode, "-t", "PNG", "-l", "L", "-m", "4", "-s", "8",
                    "-o", str(pack / name), ur,
                ], check=True)
                if (pack / name).stat().st_size == 0:
                    raise RuntimeError(f"empty QR image: {pack / name}")
                frame_names.append(name)
            title = f"Kern M5 stage {stage}, {fragment}-byte fragments"
            (pack / "index.html").write_text(
                viewer(title, frame_names, args.frame_ms), encoding="utf-8")
            digest = hashlib.sha256("\n".join(parts).encode()).hexdigest()
            rel = (pack / "index.html").relative_to(args.output).as_posix()
            links.append((title, rel))
            manifest["packs"].append({
                "stage": stage, "fragment_bytes": fragment,
                "source_parts": len(parts), "ur_lines_sha256": digest,
                "viewer": rel,
            })
    (args.output / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    items = "\n".join(
        f'<li><a href="{html.escape(path)}">{html.escape(title)}</a></li>'
        for title, path in links)
    (args.output / "index.html").write_text(
        "<!doctype html><meta charset=utf-8><title>Kern M5 physical QR packs</title>"
        "<style>body{font:18px system-ui;max-width:48rem;margin:3rem auto;line-height:1.7}</style>"
        "<h1>Kern M5 physical QR packs</h1><p>Test vectors only. No signing occurs.</p>"
        f"<ul>{items}</ul>", encoding="utf-8")
    print(args.output / "index.html")


if __name__ == "__main__":
    main()
