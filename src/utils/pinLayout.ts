/**
 * QFN 逆时针四边引脚划分工具
 */
export function getQfnLayout(pinCount: number) {
  const side = pinCount / 4;
  const left: string[] = [];
  const bottom: string[] = [];
  const right: string[] = [];
  const top: string[] = [];

  for (let i = 1; i <= side; i++) left.push(i.toString());
  for (let i = side + 1; i <= 2 * side; i++) bottom.push(i.toString());
  for (let i = 2 * side + 1; i <= 3 * side; i++) right.push(i.toString());
  for (let i = 3 * side + 1; i <= pinCount; i++) top.push(i.toString());

  return { left, bottom, right, top };
}

/**
 * BGA 排除四角引脚网格排布工具
 */
export function getBgaPins(rowsStr: string, cols: number): string[] {
  const pins: string[] = [];
  const rows = rowsStr.split('');
  const firstRow = rows[0];
  const lastRow = rows[rows.length - 1];
  const cornerExclusions = [
    `${firstRow}1`,
    `${firstRow}${cols}`,
    `${lastRow}1`,
    `${lastRow}${cols}`,
  ];

  for (const r of rows) {
    for (let c = 1; c <= cols; c++) {
      const num = `${r}${c}`;
      if (!cornerExclusions.includes(num)) {
        pins.push(num);
      }
    }
  }
  return pins;
}
