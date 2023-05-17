#!/usr/bin/env python3
import argparse
import platform
import subprocess


def main():
    parser = argparse.ArgumentParser('Bootstraps this project using conan')
    args = parser.parse_args()

    subprocess.check_call([
        'pip3',
        'install',
        'conan',
    ])

    subprocess.check_call([
        'conan',
        'profile',
        'detect',
        '--force',
    ])


if __name__ == '__main__':
    main()
