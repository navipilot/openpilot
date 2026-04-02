#!/usr/bin/env python3
"""
Example usage of the Changan RLog Tester

This demonstrates how to use the testing script programmatically
instead of via interactive input.
"""

import sys
from test_changan_with_rlog import ChanganRLogTester


def test_with_path(rlog_path: str):
    """
    Test Changan implementation with a specific rlog path.

    Args:
        rlog_path: Path to rlog directory or file

    Returns:
        0 if successful, 1 otherwise
    """
    print("="*60)
    print("长安汽车 RLog 测试示例")
    print("Changan Car RLog Testing Example")
    print("="*60)
    print(f"\n使用路径: {rlog_path}\n")

    # Create tester instance
    tester = ChanganRLogTester(rlog_path)

    # Run all tests
    success = tester.run_all_tests()

    return 0 if success else 1


def main():
    """Main entry point."""
    # Example paths (update these to your actual paths)
    example_paths = [
        "/home/jeff/realdata1/00000002--b52be56519--102",
        "/home/jeff/realdata1/00000003--a1b2c3d4e5--103",
    ]

    if len(sys.argv) > 1:
        # Use command-line argument
        rlog_path = sys.argv[1]
        return test_with_path(rlog_path)
    else:
        # Interactive mode - same as main script
        print("示例用法:")
        print("  python example_usage.py /path/to/rlog/directory")
        print()
        print("或者使用交互模式:")
        print("  python test_changan_with_rlog.py")
        print()
        print("示例路径:")
        for path in example_paths:
            print(f"  - {path}")
        return 0


if __name__ == "__main__":
    sys.exit(main())
