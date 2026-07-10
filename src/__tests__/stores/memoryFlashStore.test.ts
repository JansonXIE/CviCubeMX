// ============================================================
// CviCubeMX 重构前功能验证测试 - M5 内存配置 + M6 Flash 分区
// ============================================================

import { describe, it, expect, vi } from "vitest";
import { useMemoryStore, getDramSizeByChip } from "../../stores/memoryStore";
import { useFlashStore } from "../../stores/flashStore";

vi.mock("@tauri-apps/api/core", () => {
  const mockInvoke = vi.fn(async (cmd: string, args?: any) => {
    if (cmd === "load_memory_regions") {
      return [
        {
          name: "KERNEL_MEMORY",
          start_address: 0x80000000,
          end_address: 0x90000000,
          size: 0x10000000,
          size_string: "256M",
          is_editable: true,
          description: "Kernel",
        },
      ];
    }
    if (cmd === "load_partitions") {
      return [
        {
          partition_number: 1,
          label: "boot",
          size: 1024,
          size_string: "1M",
          file: "",
          mountpoint: "/boot",
          type_field: "ext4",
          enabled: true,
        },
      ];
    }
    if (cmd === "read_flash_board_info") {
      return {
        flash_size: "32GB",
        flash_size_kb: 32 * 1024 * 1024,
        partition_count: 9,
        partitions: [
          {
            partition_number: 2,
            label: "2nd",
            size: 3072,
            size_string: "3MB",
            file: "yoc.bin",
            mountpoint: "",
            type_field: "",
            enabled: true,
          },
          {
            partition_number: 9,
            label: "DATA",
            size: 0,
            size_string: "0KB",
            file: "data.emmc",
            mountpoint: "mnt/data",
            type_field: "ext4",
            enabled: true,
          },
        ],
      };
    }
    return Promise.resolve();
  });
  return { invoke: mockInvoke };
});

// ---- M5 参考数据 (从 memoryconfig.h 提取) ----

const TOTAL_MEMORY_SIZE = 0x10000000; // 256MB
const MEMORY_BASE_ADDRESS = 0x80000000;

// 地址格式化 (模拟 C++ formatAddress)
function formatAddress(addr: number): string {
  return "0x" + addr.toString(16).toUpperCase().padStart(8, "0");
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
  region2: { start: number; end: number },
): boolean {
  return region1.start < region2.end && region2.start < region1.end;
}

// ---- M5 测试套件 ----

describe("M5 - 内存配置 (特征化测试)", () => {
  describe("M5-T1: 总内存大小常量", () => {
    it("256MB = 0x10000000", () => {
      expect(TOTAL_MEMORY_SIZE).toBe(0x10000000);
    });
  });

  describe("M5-T2: 内存基地址常量", () => {
    it("基地址 = 0x80000000", () => {
      expect(MEMORY_BASE_ADDRESS).toBe(0x80000000);
    });
  });

  describe("M5-T3: 区域重叠校验", () => {
    it("两个重叠区域应检测到重叠", () => {
      const region1 = { start: 0x80000000, end: 0x80010000 };
      const region2 = { start: 0x80008000, end: 0x80020000 };
      expect(checkMemoryOverlap(region1, region2)).toBe(true);
    });

    it("两个不相交区域应无重叠", () => {
      const region1 = { start: 0x80000000, end: 0x80010000 };
      const region2 = { start: 0x80010000, end: 0x80020000 };
      expect(checkMemoryOverlap(region1, region2)).toBe(false);
    });

    it("包含关系应检测到重叠", () => {
      const region1 = { start: 0x80000000, end: 0x80100000 };
      const region2 = { start: 0x80010000, end: 0x80020000 };
      expect(checkMemoryOverlap(region1, region2)).toBe(true);
    });
  });

  describe("M5-T4: 地址格式化", () => {
    it("0x80000000 格式化正确", () => {
      expect(formatAddress(0x80000000)).toBe("0x80000000");
    });

    it("0x80100000 格式化正确", () => {
      expect(formatAddress(0x80100000)).toBe("0x80100000");
    });

    it("0 格式化正确", () => {
      expect(formatAddress(0)).toBe("0x00000000");
    });
  });

  describe("M5-T5: 大小格式化", () => {
    it('268435456 (256MB) → "256M"', () => {
      expect(formatSize(268435456)).toBe("256M");
    });

    it('1048576 (1MB) → "1M"', () => {
      expect(formatSize(1048576)).toBe("1M");
    });

    it('1024 (1KB) → "1K"', () => {
      expect(formatSize(1024)).toBe("1K");
    });

    it('100 (100B) → "100B"', () => {
      expect(formatSize(100)).toBe("100B");
    });

    it('0 → "0B"', () => {
      expect(formatSize(0)).toBe("0B");
    });
  });

  describe("M5-T6: JSON 导出/导入一致性", () => {
    it("导出后导入回来数据一致", () => {
      const regions = [
        {
          name: "KERNEL_MEMORY",
          startAddress: 0x80000000,
          endAddress: 0x90000000,
          size: 0x10000000,
          sizeString: "256M",
          isEditable: true,
          description: "内核",
        },
      ];
      const root = {
        memoryRegions: regions,
        totalMemorySize: "0x10000000",
        memoryBaseAddress: "0x80000000",
      };
      const jsonStr = JSON.stringify(root);
      const parsed = JSON.parse(jsonStr);
      expect(parsed.memoryRegions).toEqual(regions);
      expect(parsed.totalMemorySize).toBe("0x10000000");
    });
  });

  describe("M5-T7: defconfig 导出格式", () => {
    it("导出格式正确", () => {
      const mockLines = ["CONFIG_ION_SIZE=0x0", "CONFIG_RTOS_ION_SIZE=0x0"];
      const ionSize = 0x4b00000;
      const rtosIonSize = 0x6000000;
      const result = mockLines.map((line) => {
        if (line.startsWith("CONFIG_ION_SIZE=")) {
          return `CONFIG_ION_SIZE=0x${ionSize.toString(16)}`;
        }
        if (line.startsWith("CONFIG_RTOS_ION_SIZE=")) {
          return `CONFIG_RTOS_ION_SIZE=0x${rtosIonSize.toString(16)}`;
        }
        return line;
      });
      expect(result).toContain(`CONFIG_ION_SIZE=0x4b00000`);
      expect(result).toContain(`CONFIG_RTOS_ION_SIZE=0x6000000`);
    });
  });

  describe("M5-T8: MemoryStore (待前端实现)", () => {
    it("添加/删除区域后 store 同步", async () => {
      const store = useMemoryStore.getState();
      await store.loadMemoryRegions();
      expect(useMemoryStore.getState().regions).toHaveLength(1);

      store.addRegion({
        name: "TEST_REGION",
        start_address: 0x90000000,
        end_address: 0x95000000,
        size: 0x5000000,
        size_string: "80M",
        is_editable: true,
        description: "Test",
      });
      expect(useMemoryStore.getState().regions).toHaveLength(2);

      store.removeRegion("TEST_REGION");
      expect(useMemoryStore.getState().regions).toHaveLength(1);
    });
  });

  // ===========================================================
  // M5-T9: 不同芯片型号下 ION / RTOS_ION / RTOS_LOGO 大小修改的级联联动
  // cv1841(128MB) / cv1842(256MB) / cv1843(512MB)
  // 验证 updateRegion 按芯片 DRAM 边界（0x88000000/0x90000000/0xA0000000）联动
  // ===========================================================
  describe("M5-T9: 多芯片内存区段大小修改 (ION/RTOS_ION/RTOS_LOGO)", () => {
    const BASE = 0x80000000;
    const ION_SIZE = 0x4b00000; // 75M
    const RTOS_ION_SIZE = 0x6000000; // 96M
    const BOOTLOGO_SIZE = 0x1c2000;

    // 构造一份最小可用的区段集合（含 ION 子缓冲与 RTOS_LOGO）
    function buildRegions() {
      const mk = (name: string, start_address: number, size: number) => ({
        name,
        start_address,
        end_address: start_address + size,
        size,
        size_string: "",
        is_editable: true,
        description: "",
      });
      return [
        mk("ION", 0x85500000, ION_SIZE),
        mk("BOOTLOGO", 0x89e3e000, BOOTLOGO_SIZE),
        mk("RTOS_ION", 0x8a000000, RTOS_ION_SIZE),
        mk("H26X_BITSTREAM", 0x85500000, 0),
        mk("H26X_ENC_BUFF", 0x85500000, 0),
        mk("ISP_MEM_BASE", 0x85500000, 0),
        mk("RTOS_LOGO", 0x80580000, 0),
      ];
    }

    function setRegions() {
      useMemoryStore.setState({ regions: buildRegions() });
    }
    function region(name: string) {
      return useMemoryStore.getState().regions.find((r) => r.name === name)!;
    }

    // 每款芯片的 DRAM 容量与末尾边界
    const CHIPS: Array<{ chip: string; dram: number; endBoundary: number }> = [
      {
        chip: "cv1841cp_wevb_0015a_emmc",
        dram: 0x08000000,
        endBoundary: 0x88000000,
      },
      {
        chip: "cv1842hp_wevb_0014a_emmc",
        dram: 0x10000000,
        endBoundary: 0x90000000,
      },
      {
        chip: "cv1843hp_wevb_0014a_emmc",
        dram: 0x20000000,
        endBoundary: 0xa0000000,
      },
    ];

    it("getDramSizeByChip 按型号前缀解析 DRAM 容量", () => {
      expect(getDramSizeByChip("cv1841cp_wevb_0015a_emmc")).toBe(0x08000000);
      expect(getDramSizeByChip("cv1842hp_wevb_0014a_emmc")).toBe(0x10000000);
      expect(getDramSizeByChip("cv1843hp_wevb_0014a_emmc")).toBe(0x20000000);
      // 未识别/未指定回退到默认 256M
      expect(getDramSizeByChip("cv1840cp_wevb_0015a_spinor")).toBe(0x10000000);
      expect(getDramSizeByChip(undefined)).toBe(0x10000000);
    });

    // ---- 修改 RTOS_ION 大小：end 钉死在各芯片 DDR 末尾，并联动 ION ----
    describe("修改 RTOS_ION 大小（end 钉死在 DDR 末尾）", () => {
      it.each(CHIPS)(
        "$chip: RTOS_ION/ION 起止地址随 DRAM 边界平移",
        ({ chip, endBoundary }) => {
          setRegions();
          useMemoryStore
            .getState()
            .updateRegion("RTOS_ION", { size: RTOS_ION_SIZE }, chip);

          const rtosIon = region("RTOS_ION");
          // 结束地址恒为该芯片的 DDR 末尾
          expect(rtosIon.end_address).toBe(endBoundary);
          expect(rtosIon.start_address).toBe(endBoundary - RTOS_ION_SIZE);

          // ION 紧贴 RTOS_ION 之下
          const ion = region("ION");
          expect(ion.end_address).toBe(rtosIon.start_address);
          expect(ion.start_address).toBe(rtosIon.start_address - ION_SIZE);

          // BOOTLOGO 位于 ION 尾部，end 与 ION end / RTOS_ION start 保持一致
          const bootlogo = region("BOOTLOGO");
          expect(bootlogo.end_address).toBe(ion.end_address);
          expect(bootlogo.end_address).toBe(rtosIon.start_address);
          expect(bootlogo.start_address).toBe(
            bootlogo.end_address - bootlogo.size,
          );

          // 子缓冲与 ION 共享起始地址
          for (const sub of [
            "H26X_BITSTREAM",
            "H26X_ENC_BUFF",
            "ISP_MEM_BASE",
          ]) {
            expect(region(sub).start_address).toBe(ion.start_address);
          }
        },
      );

      it("cv1842(256M): 与默认布局精确吻合", () => {
        setRegions();
        useMemoryStore
          .getState()
          .updateRegion(
            "RTOS_ION",
            { size: RTOS_ION_SIZE },
            "cv1842hp_wevb_0014a_emmc",
          );
        expect(region("RTOS_ION").start_address).toBe(0x8a000000);
        expect(region("ION").start_address).toBe(0x85500000);
        expect(region("ION").end_address).toBe(0x8a000000);
        expect(region("BOOTLOGO").start_address).toBe(0x89e3e000);
        expect(region("BOOTLOGO").end_address).toBe(0x8a000000);
      });

      it("cv1843(512M): RTOS_ION 上移到 0x9A000000，ION 上移到 0x95500000", () => {
        setRegions();
        useMemoryStore
          .getState()
          .updateRegion(
            "RTOS_ION",
            { size: RTOS_ION_SIZE },
            "cv1843hp_wevb_0014a_emmc",
          );
        expect(region("RTOS_ION").start_address).toBe(0x9a000000);
        expect(region("ION").start_address).toBe(0x95500000);
        expect(region("ION").end_address).toBe(0x9a000000);
        expect(region("BOOTLOGO").end_address).toBe(0x9a000000);
        expect(region("BOOTLOGO").start_address).toBe(
          0x9a000000 - BOOTLOGO_SIZE,
        );
      });

      it("cv1841(128M): 默认 ION+RTOS_ION(171M) 超出 128M，ION 起始跌破基址（已知约束，需调小尺寸）", () => {
        setRegions();
        useMemoryStore
          .getState()
          .updateRegion(
            "RTOS_ION",
            { size: RTOS_ION_SIZE },
            "cv1841cp_wevb_0015a_emmc",
          );
        expect(region("RTOS_ION").start_address).toBe(0x82000000);
        // 75M+96M=171M > 128M，ION 被挤到基址 0x80000000 之下
        expect(region("ION").start_address).toBe(0x7d500000);
        expect(region("ION").start_address).toBeLessThan(BASE);
      });

      it("cv1841(128M): 调小 ION/RTOS_ION 后布局回到合法范围", () => {
        // 32M + 32M = 64M，可容于 128M
        const small = 0x2000000;
        useMemoryStore.setState({
          regions: [
            {
              name: "ION",
              start_address: 0x84000000,
              end_address: 0x86000000,
              size: small,
              size_string: "",
              is_editable: true,
              description: "",
            },
            {
              name: "RTOS_ION",
              start_address: 0x86000000,
              end_address: 0x88000000,
              size: small,
              size_string: "",
              is_editable: true,
              description: "",
            },
          ],
        });
        useMemoryStore
          .getState()
          .updateRegion(
            "RTOS_ION",
            { size: small },
            "cv1841cp_wevb_0015a_emmc",
          );
        expect(region("RTOS_ION").end_address).toBe(0x88000000);
        expect(region("RTOS_ION").start_address).toBe(0x86000000);
        expect(region("ION").start_address).toBe(0x84000000);
        expect(region("ION").start_address).toBeGreaterThanOrEqual(BASE);
      });
    });

    // ---- 修改 ION 大小：end = RTOS_ION.start，向低地址扩展 ----
    describe("修改 ION 大小（end 对齐 RTOS_ION 起始）", () => {
      it.each(CHIPS)(
        "$chip: ION 扩到 80M 后起始地址正确",
        ({ chip, endBoundary }) => {
          setRegions();
          // 先让 RTOS_ION 处于该芯片正确位置
          useMemoryStore
            .getState()
            .updateRegion("RTOS_ION", { size: RTOS_ION_SIZE }, chip);
          const rtosIonStart = endBoundary - RTOS_ION_SIZE;

          const newIon = 0x5000000; // 80M
          useMemoryStore.getState().updateRegion("ION", { size: newIon }, chip);

          const ion = region("ION");
          expect(ion.end_address).toBe(rtosIonStart);
          expect(ion.start_address).toBe(rtosIonStart - newIon);

          const bootlogoAfterIonResize = region("BOOTLOGO");
          expect(bootlogoAfterIonResize.end_address).toBe(ion.end_address);
          expect(bootlogoAfterIonResize.start_address).toBe(
            bootlogoAfterIonResize.end_address - bootlogoAfterIonResize.size,
          );
          // 子缓冲跟随 ION 起始
          expect(region("H26X_BITSTREAM").start_address).toBe(
            ion.start_address,
          );
          expect(region("ISP_MEM_BASE").start_address).toBe(ion.start_address);
        },
      );

      it("无 RTOS_ION 时走回退分支：end = endBoundary - 96M（随芯片不同）", () => {
        const mkIon = () =>
          useMemoryStore.setState({
            regions: [
              {
                name: "ION",
                start_address: 0x85500000,
                end_address: 0x8a000000,
                size: ION_SIZE,
                size_string: "",
                is_editable: true,
                description: "",
              },
            ],
          });
        mkIon();
        useMemoryStore
          .getState()
          .updateRegion("ION", { size: ION_SIZE }, "cv1842hp_wevb_0014a_emmc");
        expect(region("ION").end_address).toBe(0x90000000 - 96 * 1024 * 1024); // 0x8a000000
        mkIon();
        useMemoryStore
          .getState()
          .updateRegion("ION", { size: ION_SIZE }, "cv1843hp_wevb_0014a_emmc");
        expect(region("ION").end_address).toBe(0xa0000000 - 96 * 1024 * 1024); // 0x9a000000
      });
    });

    // ---- 修改 BOOTLOGO 大小：end贴住 ION 尾部，start 根据 size 回推 ----
    describe("修改 BOOTLOGO 大小（end 对齐 ION末尾）", () => {
      it("cv1842(256M): BOOTLOGO size 修改后仍贴住 ION end", () => {
        setRegions();
        useMemoryStore
          .getState()
          .updateRegion(
            "RTOS_ION",
            { size: RTOS_ION_SIZE },
            "cv1842hp_wevb_0014a_emmc",
          );

        const newSize = 0x200000;
        useMemoryStore
          .getState()
          .updateRegion(
            "BOOTLOGO",
            { size: newSize },
            "cv1842hp_wevb_0014a_emmc",
          );

        const ion = region("ION");
        const bootlogo = region("BOOTLOGO");
        expect(bootlogo.size).toBe(newSize);
        expect(bootlogo.end_address).toBe(ion.end_address);
        expect(bootlogo.start_address).toBe(ion.end_address - newSize);
      });
    });

    // ---- 修改 RTOS_LOGO 大小：普通区段，仅 end = start + size，不随 DRAM 边界移动 ----
    describe("修改 RTOS_LOGO 大小（普通区段，简单重算 end）", () => {
      it.each(CHIPS)(
        "$chip: RTOS_LOGO 起始不变，end = start + size",
        ({ chip }) => {
          setRegions();
          const newSize = 0x100000; // 1M
          useMemoryStore
            .getState()
            .updateRegion(
              "RTOS_LOGO",
              { start_address: 0x80580000, size: newSize },
              chip,
            );
          const logo = region("RTOS_LOGO");
          // 普通区段不被 DDR 末尾边界影响
          expect(logo.start_address).toBe(0x80580000);
          expect(logo.end_address).toBe(0x80580000 + newSize);
        },
      );
    });
  });
});

// ---- M6 参考数据 ----

interface FlashPartitionRef {
  partitionNumber: number;
  label: string;
  size: number; // KB
  file: string;
  mountpoint: string;
  type: string;
  enabled: boolean;
}

// 分区大小校验
function validatePartitionLayout(
  partitions: FlashPartitionRef[],
  flashSizeKB: number,
): { valid: boolean; totalSize: number } {
  const totalSize = partitions
    .filter((p) => p.enabled)
    .reduce((sum, p) => sum + p.size, 0);
  return { valid: totalSize <= flashSizeKB, totalSize };
}

// ---- M6 测试套件 ----

describe("M6 - Flash 分区管理 (特征化测试)", () => {
  describe("M6-T1: 分区大小校验", () => {
    it("分区总大小 <= Flash 总容量时校验通过", () => {
      const partitions: FlashPartitionRef[] = [
        {
          partitionNumber: 1,
          label: "boot",
          size: 1024,
          file: "",
          mountpoint: "/boot",
          type: "ext4",
          enabled: true,
        },
        {
          partitionNumber: 2,
          label: "rootfs",
          size: 30720,
          file: "",
          mountpoint: "/",
          type: "ext4",
          enabled: true,
        },
      ];
      const result = validatePartitionLayout(partitions, 32768);
      expect(result.valid).toBe(true);
    });

    it("分区总大小超过 Flash 总容量时校验失败", () => {
      const partitions: FlashPartitionRef[] = [
        {
          partitionNumber: 1,
          label: "boot",
          size: 1024,
          file: "",
          mountpoint: "/boot",
          type: "ext4",
          enabled: true,
        },
        {
          partitionNumber: 2,
          label: "rootfs",
          size: 65536,
          file: "",
          mountpoint: "/",
          type: "ext4",
          enabled: true,
        },
      ];
      const result = validatePartitionLayout(partitions, 32768);
      expect(result.valid).toBe(false);
    });

    it("禁用的分区不应计入总大小", () => {
      const partitions: FlashPartitionRef[] = [
        {
          partitionNumber: 1,
          label: "boot",
          size: 1024,
          file: "",
          mountpoint: "/boot",
          type: "ext4",
          enabled: true,
        },
        {
          partitionNumber: 2,
          label: "data",
          size: 999999,
          file: "",
          mountpoint: "/data",
          type: "ext4",
          enabled: false,
        },
      ];
      const result = validatePartitionLayout(partitions, 32768);
      expect(result.valid).toBe(true);
      expect(result.totalSize).toBe(1024);
    });
  });

  describe("M6-T2: 分区增删", () => {
    it("添加分区后列表长度增加", () => {
      const partitions: FlashPartitionRef[] = [
        {
          partitionNumber: 1,
          label: "boot",
          size: 1024,
          file: "",
          mountpoint: "/boot",
          type: "ext4",
          enabled: true,
        },
      ];
      partitions.push({
        partitionNumber: 2,
        label: "rootfs",
        size: 30720,
        file: "",
        mountpoint: "/",
        type: "ext4",
        enabled: true,
      });
      expect(partitions.length).toBe(2);
    });

    it("删除分区后列表长度减少", () => {
      const partitions: FlashPartitionRef[] = [
        {
          partitionNumber: 1,
          label: "boot",
          size: 1024,
          file: "",
          mountpoint: "/boot",
          type: "ext4",
          enabled: true,
        },
        {
          partitionNumber: 2,
          label: "rootfs",
          size: 30720,
          file: "",
          mountpoint: "/",
          type: "ext4",
          enabled: true,
        },
      ];
      partitions.splice(0, 1);
      expect(partitions.length).toBe(1);
      expect(partitions[0].label).toBe("rootfs");
    });
  });

  describe("M6-T3: JSON 导出/导入一致性", () => {
    it("导出后导入回来数据一致", () => {
      const partitions = [
        {
          partitionNumber: 2,
          label: "BOOT",
          size: 8192,
          file: "boot.emmc",
          mountpoint: "",
          type: "",
          enabled: true,
        },
      ];
      const jsonStr = JSON.stringify({
        flashSize: "32GB",
        partitionCount: 9,
        partitions,
      });
      const parsed = JSON.parse(jsonStr);
      expect(parsed.partitions).toEqual(partitions);
      expect(parsed.flashSize).toBe("32GB");
    });
  });

  describe("M6-T4: defconfig 导出格式", () => {
    it("导出格式正确", () => {
      const mockLines = [
        "CONFIG_PARTITION_2=y",
        'CONFIG_PARTITION_2_LABEL="2nd"',
        'CONFIG_PARTITION_2_SIZE="3072"',
      ];
      const partition = {
        partitionNumber: 2,
        label: "BOOT_NEW",
        size: 4096,
        enabled: true,
      };
      const result = mockLines.map((line) => {
        if (
          line.startsWith(
            `CONFIG_PARTITION_${partition.partitionNumber}_LABEL=`,
          )
        ) {
          return `CONFIG_PARTITION_${partition.partitionNumber}_LABEL="${partition.label}"`;
        }
        if (
          line.startsWith(`CONFIG_PARTITION_${partition.partitionNumber}_SIZE=`)
        ) {
          return `CONFIG_PARTITION_${partition.partitionNumber}_SIZE="${partition.size}"`;
        }
        return line;
      });
      expect(result).toContain(`CONFIG_PARTITION_2_LABEL="BOOT_NEW"`);
      expect(result).toContain(`CONFIG_PARTITION_2_SIZE="4096"`);
    });
  });

  describe("M6-T5: FlashStore (待前端实现)", () => {
    it("添加/删除分区后 store 同步", async () => {
      const store = useFlashStore.getState();
      await store.loadPartitions();
      expect(useFlashStore.getState().partitions).toHaveLength(1);

      store.addPartition({
        partition_number: 2,
        label: "rootfs",
        size: 30720,
        size_string: "30M",
        file: "",
        mountpoint: "/",
        type_field: "ext4",
        enabled: true,
      });
      expect(useFlashStore.getState().partitions).toHaveLength(2);

      store.removePartition(2);
      expect(useFlashStore.getState().partitions).toHaveLength(1);
    });
  });

  describe("M6-T6: 从 SDK 板卡读取分区信息", () => {
    it("loadBoardInfo 写入分区表与 Flash 容量", async () => {
      const store = useFlashStore.getState();
      await store.loadBoardInfo("/fake/sdk", "cv1842hp_wevb_0014a_emmc");

      const state = useFlashStore.getState();
      expect(state.flashSize).toBe("32GB");
      expect(state.flashSizeKb).toBe(32 * 1024 * 1024);
      expect(state.partitionCount).toBe(9);
      expect(state.partitions).toHaveLength(2);
      expect(state.partitions[0].label).toBe("2nd");
      // DATA 分区 size 为 0 => 自动分配
      expect(state.partitions[1].label).toBe("DATA");
      expect(state.partitions[1].size).toBe(0);
    });
  });

  describe("M6-T7: 修改分区大小", () => {
    it("updatePartitionSize 只改对应分区的 size 与 size_string", async () => {
      const store = useFlashStore.getState();
      await store.loadBoardInfo("/fake/sdk", "cv1842hp_wevb_0014a_emmc");

      store.updatePartitionSize(2, 8192);
      let p2 = useFlashStore
        .getState()
        .partitions.find((p) => p.partition_number === 2)!;
      expect(p2.size).toBe(8192);
      expect(p2.size_string).toBe("8MB");

      // 留空（size 0）=> 自动分配
      store.updatePartitionSize(2, 0);
      p2 = useFlashStore
        .getState()
        .partitions.find((p) => p.partition_number === 2)!;
      expect(p2.size).toBe(0);
    });
  });

  describe("M6-T8: 重置恢复载入基线", () => {
    it("修改并（模拟）保存后，重置恢复到载入时的分区大小", async () => {
      const store = useFlashStore.getState();
      await store.loadBoardInfo("/fake/sdk", "cv1842hp_wevb_0014a_emmc");
      const original = useFlashStore
        .getState()
        .partitions.find((p) => p.partition_number === 2)!.size;
      expect(original).toBe(3072);

      // 用户改大小并“保存”（保存只写磁盘，不影响内存基线）
      store.updatePartitionSize(2, 4096);
      expect(
        useFlashStore
          .getState()
          .partitions.find((p) => p.partition_number === 2)!.size,
      ).toBe(4096);

      // 重置 => 恢复到载入时的 3072（而非已保存的 4096）
      store.resetToBaseline();
      expect(
        useFlashStore
          .getState()
          .partitions.find((p) => p.partition_number === 2)!.size,
      ).toBe(original);
    });
  });
});
