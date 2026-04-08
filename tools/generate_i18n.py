#!/usr/bin/env python3
"""
Generate i18n YAML files from CSV file
Usage: python generate_i18n.py
"""

import csv
import os
from pathlib import Path

def escape_yaml_string(value):
    """Escape special characters for YAML"""
    if not value:
        return '""'
    
    # Check if string needs quoting
    needs_quote = any(c in value for c in [':', '#', '\n', ',', '%', '*'])
    
    if needs_quote:
        # Escape quotes and wrap in double quotes
        escaped = value.replace('"', '\\"')
        return f'"{escaped}"'
    
    return value

def generate_yml_files():
    """Generate en.yml, zh-cn.yml, and ko.yml from i18n.csv"""
    
    # Get paths
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    i18n_dir = project_root / 'src' / 'ui' / 'i18n'
    csv_file = i18n_dir / 'i18n.csv'
    
    if not csv_file.exists():
        print(f"Error: {csv_file} not found")
        return
    
    # Read CSV
    translations = []
    with open(csv_file, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            translations.append(row)
    
    # Group by comment for better organization
    grouped = {}
    for trans in translations:
        comment = trans['comment']
        if comment not in grouped:
            grouped[comment] = []
        grouped[comment].append(trans)
    
    # Generate en.yml
    en_file = i18n_dir / 'en.yml'
    with open(en_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write("'en':\n")
        for comment, items in grouped.items():
            f.write(f"  # {comment}\n")
            for item in items:
                key = item['key']
                value = escape_yaml_string(item['en'])
                f.write(f"  {key}: {value}\n")
            f.write("\n")
    
    # Generate zh-cn.yml
    zh_file = i18n_dir / 'zh-cn.yml'
    with open(zh_file, 'w', encoding='utf-8', newline='\n') as f:
        f.write("'zh-cn':\n")
        for comment, items in grouped.items():
            # Translate comment categories to Chinese
            comment_cn_map = {
                'Home page': '主页',
                'Config page': '配置页',
                'Speed control page': '速度控制页',
                'Widget color page': '颜色组件页',
                'LCD brightness page': 'LCD亮度页',
                'System page': '系统页',
                'Firmware page': '固件页',
                'Brake tile': '刹车磁贴',
                'Essentials tile': '基础磁贴',
                'Motor1 tile': '电机1磁贴',
                'Motor2 tile': '电机2磁贴',
                'Motor3 tile': '电机3磁贴',
                'Motor4 tile': '电机4磁贴',
                'Limits tile': '限制磁贴',
                'PID tile': 'PID磁贴',
                'Servo tile': '舵机磁贴',
                'Sin tile': '正弦磁贴',
                'Firmware tile': '固件磁贴',
            }
            comment_cn = comment_cn_map.get(comment, comment)
            f.write(f"  # {comment_cn}\n")
            for item in items:
                key = item['key']
                value = escape_yaml_string(item['zh-cn'])
                f.write(f"  {key}: {value}\n")
            f.write("\n")

    # Generate ko.yml (if column exists)
    if translations and 'ko' in translations[0]:
        ko_file = i18n_dir / 'ko.yml'
        with open(ko_file, 'w', encoding='utf-8', newline='\n') as f:
            f.write("'ko':\n")
            comment_ko_map = {
                'Home page': '홈 페이지',
                'Config page': '설정 페이지',
                'Speed control page': '속도 제어 페이지',
                'Widget color page': '위젯 색상 페이지',
                'LCD brightness page': 'LCD 밝기 페이지',
                'System page': '시스템 페이지',
                'Firmware page': '펌웨어 페이지',
                'Brake tile': '브레이크 타일',
                'Essentials tile': '필수 타일',
                'Motor1 tile': '모터1 타일',
                'Motor2 tile': '모터2 타일',
                'Motor3 tile': '모터3 타일',
                'Motor4 tile': '모터4 타일',
                'Limits tile': '제한 타일',
                'PID tile': 'PID 타일',
                'Servo tile': '서보 타일',
                'Sin tile': '사인 타일',
                'Firmware tile': '펌웨어 타일',
                'Language page': '언어 페이지',
                'About page': '정보 페이지',
            }
            for comment, items in grouped.items():
                comment_ko = comment_ko_map.get(comment, comment)
                f.write(f"  # {comment_ko}\n")
                for item in items:
                    key = item['key']
                    value = escape_yaml_string(item['ko'])
                    f.write(f"  {key}: {value}\n")
                f.write("\n")
    
    print(f"Generated: {en_file}")
    print(f"Generated: {zh_file}")
    if translations and 'ko' in translations[0]:
        print(f"Generated: {ko_file}")
    print(f"Total translations: {len(translations)}")

if __name__ == '__main__':
    generate_yml_files()
