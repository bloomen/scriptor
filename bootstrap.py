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
        'conan==1.45.0',
    ])

    try:
        subprocess.check_call([
            'conan',
            'profile',
            'new',
            'default',
            '--detect',
        ], stderr=subprocess.PIPE)
    except BaseException as e:
        pass

    if platform.system() == 'Linux':
        subprocess.check_call([
            'conan',
            'profile',
            'update',
            'settings.compiler.libcxx=libstdc++11',
            'default',
        ])


if __name__ == '__main__':
    main()
