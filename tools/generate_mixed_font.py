#!/usr/bin/env python3
"""
Generate mixed font with Montserrat (English) and SourceHanSans (Chinese)
Usage: python generate_mixed_font.py <chinese_chars> <output_file>
"""

import sys
import os
import subprocess

def generate_mixed_font(cjk_chars, output_file):
    """Generate font with English from Montserrat, Chinese from SourceHanSansSC, and Korean from SourceHanSansK"""
    
    # Get script directory and project root
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    # Use UI/i18n fonts only (avoid LVGL built_in_font)
    font_dir = os.path.join(project_root, "src", "ui", "i18n")
    
    # Font files (use relative names, will change to font_dir before running)
    montserrat_font = "Montserrat-Medium.ttf"
    # lv_font_conv doesn't support variable fonts; use static OTF
    chinese_font = "SourceHanSansSC-Normal.otf"
    korean_font = "SourceHanSansK-Normal.otf"
    icon_font = "FontAwesome5-Solid+Brands+Regular.woff"
    
    # Convert output file to absolute path
    output_file_abs = os.path.abspath(output_file)
    
    # Validate required fonts exist in ui/i18n
    required_fonts = [montserrat_font, chinese_font, korean_font, icon_font]
    missing_fonts = [f for f in required_fonts if not os.path.exists(os.path.join(font_dir, f))]
    if missing_fonts:
        print("Error: Missing fonts in src/ui/i18n:")
        for f in missing_fonts:
            print(f"  - {f}")
        print("Please copy the missing font files into src/ui/i18n and retry.")
        sys.exit(1)

    # Split CJK characters by unicode ranges
    korean_chars_set = set(ch for ch in cjk_chars if ('\u1100' <= ch <= '\u11ff') or ('\u3130' <= ch <= '\u318f') or ('\uac00' <= ch <= '\ud7a3'))
    korean_chars = ''.join(sorted(korean_chars_set))
    chinese_chars = ''.join(sorted(ch for ch in cjk_chars if ch not in korean_chars_set))

    # Built-in FontAwesome symbols
    icon_range = "61441,61448,61451,61452,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"
    
    # Build lv_font_conv command
    cmd = [
        "npx", "lv_font_conv",
        "--no-compress", "--no-prefilter",
        "--bpp", "4",
        "--size", "14",
        # Montserrat for ASCII and degree symbol
        "--font", montserrat_font,
        "-r", "0x20-0x7E,0xB0",
    ]

    if chinese_chars:
        cmd += [
            "--font", chinese_font,
            "--symbols", chinese_chars,
        ]

    if korean_chars:
        cmd += [
            "--font", korean_font,
            "--symbols", korean_chars,
        ]

    cmd += [
        # FontAwesome for icons
        "--font", icon_font,
        "-r", icon_range,
        "--format", "lvgl",
        "-o", output_file_abs,
        "--force-fast-kern-format"
    ]
    
    print(f"Generating mixed font...")
    print(f"Command: {' '.join(cmd)}")
    
    # Change to font directory before running command to avoid absolute paths in output
    original_cwd = os.getcwd()
    os.chdir(font_dir)
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, shell=True)

        if result.returncode != 0:
            print("Font generation failed. Output:")
            if result.stdout:
                print(result.stdout)
            if result.stderr:
                print(result.stderr)
            sys.exit(1)

        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr)
        print(f"Font generated successfully: {output_file}")
    finally:
        # Restore original working directory
        os.chdir(original_cwd)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python generate_mixed_font.py <cjk_chars_or_file> <output_file>")
        sys.exit(1)

    cjk_arg = sys.argv[1]
    output_file = sys.argv[2]

    if os.path.isfile(cjk_arg):
        with open(cjk_arg, 'r', encoding='utf-8') as f:
            cjk_chars = f.read()
    else:
        cjk_chars = cjk_arg

    generate_mixed_font(cjk_chars, output_file)
