#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import yaml
import re
import sys

def extract_cjk_chars(yaml_files):
    """从YAML文件中提取所有中日韩字符"""
    try:
        cjk_chars = set()

        def extract_from_obj(obj):
            if isinstance(obj, str):
                # 匹配中日韩字符（包含常用标点）
                chars = re.findall(r'[\u4e00-\u9fff\u3000-\u303f\uff00-\uffef\u1100-\u11ff\u3130-\u318f\uac00-\ud7a3]', obj)
                cjk_chars.update(chars)
            elif isinstance(obj, dict):
                for value in obj.values():
                    extract_from_obj(value)
            elif isinstance(obj, list):
                for item in obj:
                    extract_from_obj(item)

        for yaml_file in yaml_files:
            with open(yaml_file, 'r', encoding='utf-8') as f:
                data = yaml.safe_load(f)
            extract_from_obj(data)

        # 转换为排序后的字符串
        symbols = ''.join(sorted(cjk_chars))
        return symbols
    
    except FileNotFoundError:
        print(f"Error: File '{yaml_file}' not found", file=sys.stderr)
        sys.exit(1)
    except yaml.YAMLError as e:
        print(f"Error parsing YAML: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python extract_chinese_chars.py <yaml_file> [yaml_file...]")
        sys.exit(1)

    yaml_files = sys.argv[1:]
    symbols = extract_cjk_chars(yaml_files)

    # 输出提取的字符（不换行），以 UTF-8 编码避免 Windows 控制台编码问题
    sys.stdout.buffer.write(symbols.encode('utf-8'))
