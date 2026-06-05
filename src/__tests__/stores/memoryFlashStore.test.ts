// ============================================================
// CviCubeMX 重构前功能验证测试 - M5 内存配置 + M6 Flash 分区
// ============================================================

import { describe, it, expect } from 'vitest';

// ---- M5 参考数据 (从 memoryconfig.h 提取) ----

const TOTAL_MEMORY_SIZE = 0x10000000;  // 256MB
const MEMORY_BASE_ADDRESS = 0x80000000;

// 地址格式化 (模拟 C++ formatAddress)
function formatAddress(addr: number): string {
  return '0x' + addr.toString(16).toUpperCase().padStart(8, '0');
}

// 大小格式化 (模拟 C++ formatSize)
function formatSize(sizeInBytes: number): string {
  if (sizeInBytes >= 1024 * 1024) {
    const mb = sizeInBytes / (1024 * 1024);
    return Number.isInteger(mb) ? `${mb}M` : `${mb.toFixed(1)}M`;
  } else if (sizeInBytes >= 1024) {
    const kb = sizeInBytes / 1024;
    return Number.isInteger(kb) ? `${kb}K` : `${kb.toFixed(1)}K`;
  }
  return `${sizeInBytes}B`;
}

// 内存重叠检测
function checkMemoryOverlap(
  region1: { start: number; end: number },
  region2: { start: number; end: number }
): boolean {
  return region1.start < region2.end && region2.start < region1.end;
}

// ---- M5 测试套件 ----

describe('M5 - 内存配置 (特征化测试)', () => {

  describe('M5-T1: 总内存大小常量', () => {
    it('256MB = 0x10000000', () => {
      expect(TOTAL_MEMORY_SIZE).toBe(0x10000000);
    });
  });

  describe('M5-T2: 内存基地址常量', () => {
    it('基地址 = 0x80000000', () => {
      expect(MEMORY_BASE_ADDRESS).toBe(0x80000000);
    });
  });

  describe('M5-T3: 区域重叠校验', () => {
    it('两个重叠区域应检测到重叠', () => {
      const region1 = { start: 0x80000000, end: 0x80010000 };
      const region2 = { start: 0x80008000, end: 0x80020000 };
      expect(checkMemoryOverlap(region1, region2)).toBe(true);
    });

    it('两个不相交区域应无重叠', () => {
      const region1 = { start: 0x80000000, end: 0x80010000 };
      const region2 = { start: 0x80010000, end: 0x80020000 };
      expect(checkMemoryOverlap(region1, region2)).toBe(false);
    });

    it('包含关系应检测到重叠', () => {
      const region1 = { start: 0x80000000, end: 0x80100000 };
      const region2 = { start: 0x80010000, end: 0x80020000 };
      expect(checkMemoryOverlap(region1, region2)).toBe(true);
    });
  });

  describe('M5-T4: 地址格式化', () => {
    it('0x80000000 格式化正确', () => {
      expect(formatAddress(0x80000000)).toBe('0x80000000');
    });

    it('0x80100000 格式化正确', () => {
      expect(formatAddress(0x80100000)).toBe('0x80100000');
    });

    it('0 格式化正确', () => {
      expect(formatAddress(0)).toBe('0x00000000');
    });
  });

  describe('M5-T5: 大小格式化', () => {
    it('268435456 (256MB) → "256M"', () => {
      expect(formatSize(268435456)).toBe('256M');
    });

    it('1048576 (1MB) → "1M"', () => {
      expect(formatSize(1048576)).toBe('1M');
    });

    it('1024 (1KB) → "1K"', () => {
      expect(formatSize(1024)).toBe('1K');
    });

    it('100 (100B) → "100B"', () => {
      expect(formatSize(100)).toBe('100B');
    });

    it('0 → "0B"', () => {
      expect(formatSize(0)).toBe('0B');
    });
  });

  describe('M5-T6: JSON 导出/导入一致性', () => {
    it.skip('导出后导入回来数据一致', () => {
      // 待 Rust 实现后验证
    });
  });

  describe('M5-T7: defconfig 导出格式', () => {
    it.skip('导出格式正确', () => {
      // 待 Rust 实现后验证
    });
  });

  describe('M5-T8: MemoryStore (待前端实现)', () => {
    it.skip('添加/删除区域后 store 同步', () => {});
  });
});

// ---- M6 参考数据 ----

interface FlashPartitionRef {
  partitionNumber: number;
  label: string;
  size: number;  // KB
  file: string;
  mountpoint: string;
  type: string;
  enabled: boolean;
}

// 分区大小校验
function validatePartitionLayout(partitions: FlashPartitionRef[], flashSizeKB: number): { valid: boolean; totalSize: number } {
  const totalSize = partitions.filter(p => p.enabled).reduce((sum, p) => sum + p.size, 0);
  return { valid: totalSize <= flashSizeKB, totalSize };
}

// ---- M6 测试套件 ----

describe('M6 - Flash 分区管理 (特征化测试)', () => {

  describe('M6-T1: 分区大小校验', () => {
    it('分区总大小 <= Flash 总容量时校验通过', () => {
      const partitions: FlashPartitionRef[] = [
        { partitionNumber: 1, label: 'boot', size: 1024, file: '', mountpoint: '/boot', type: 'ext4', enabled: true },
        { partitionNumber: 2, label: 'rootfs', size: 30720, file: '', mountpoint: '/', type: 'ext4', enabled: true },
      ];
      const result = validatePartitionLayout(partitions, 32768);
      expect(result.valid).toBe(true);
    });

    it('分区总大小超过 Flash 总容量时校验失败', () => {
      const partitions: FlashPartitionRef[] = [
        { partitionNumber: 1, label: 'boot', size: 1024, file: '', mountpoint: '/boot', type: 'ext4', enabled: true },
        { partitionNumber: 2, label: 'rootfs', size: 65536, file: '', mountpoint: '/', type: 'ext4', enabled: true },
      ];
      const result = validatePartitionLayout(partitions, 32768);
      expect(result.valid).toBe(false);
    });

    it('禁用的分区不应计入总大小', () => {
      const partitions: FlashPartitionRef[] = [
        { partitionNumber: 1, label: 'boot', size: 1024, file: '', mountpoint: '/boot', type: 'ext4', enabled: true },
        { partitionNumber: 2, label: 'data', size: 999999, file: '', mountpoint: '/data', type: 'ext4', enabled: false },
      ];
      const result = validatePartitionLayout(partitions, 32768);
      expect(result.valid).toBe(true);
      expect(result.totalSize).toBe(1024);
    });
  });

  describe('M6-T2: 分区增删', () => {
    it('添加分区后列表长度增加', () => {
      const partitions: FlashPartitionRef[] = [
        { partitionNumber: 1, label: 'boot', size: 1024, file: '', mountpoint: '/boot', type: 'ext4', enabled: true },
      ];
      partitions.push({ partitionNumber: 2, label: 'rootfs', size: 30720, file: '', mountpoint: '/', type: 'ext4', enabled: true });
      expect(partitions.length).toBe(2);
    });

    it('删除分区后列表长度减少', () => {
      const partitions: FlashPartitionRef[] = [
        { partitionNumber: 1, label: 'boot', size: 1024, file: '', mountpoint: '/boot', type: 'ext4', enabled: true },
        { partitionNumber: 2, label: 'rootfs', size: 30720, file: '', mountpoint: '/', type: 'ext4', enabled: true },
      ];
      partitions.splice(0, 1);
      expect(partitions.length).toBe(1);
      expect(partitions[0].label).toBe('rootfs');
    });
  });

  describe('M6-T3: JSON 导出/导入一致性', () => {
    it.skip('导出后导入回来数据一致', () => {});
  });

  describe('M6-T4: defconfig 导出格式', () => {
    it.skip('导出格式正确', () => {});
  });

  describe('M6-T5: FlashStore (待前端实现)', () => {
    it.skip('添加/删除分区后 store 同步', () => {});
  });
});