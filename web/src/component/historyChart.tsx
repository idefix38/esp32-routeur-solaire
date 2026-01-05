import React, { useMemo, useState, useRef } from 'react';

// Lightweight dependency-free HistoryChart.
// Props:
// - data: {time:number, value:number}[] (time in seconds)
// - minY/maxY: numeric range
// - yStep: step between Y ticks (default 10)
// - xStep: step on X axis in seconds (default 4h)
// - lineClass/labelClass: optional Tailwind classes (preferred)
// - tooltipClass: optional Tailwind class for tooltip styling

interface DataPoint { time: number; value: number }

interface HistoryChartProps {
  data: DataPoint[];
  minY: number;
  maxY: number;
  height?: number;
  yStep?: number;
  xStep?: number; // seconds
  lineClass?: string;
  labelClass?: string;
  tooltipClass?: string;
  // background class for tooltip (Tailwind), default indigo-600
  tooltipBgClass?: string;
  labelTemplate?: string;
}

const defaultXStep = 4 * 3600;

const formatTime = (tsSeconds: number) => {
  const d = new Date(tsSeconds * 1000);
  return d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
};

function linearScale(domain: [number, number], range: [number, number]) {
  const [d0, d1] = domain;
  const [r0, r1] = range;
  const span = d1 - d0 || 1;
  const outSpan = r1 - r0;
  return (v: number) => r0 + ((v - d0) / span) * outSpan;
}

const HistoryChart: React.FC<HistoryChartProps> = ({
  data,
  minY,
  maxY,
  height = 160,
  yStep = 10,
  xStep = defaultXStep,
  lineClass,
  labelClass,
  tooltipClass,
  tooltipBgClass = 'bg-indigo-600',
  labelTemplate = '{time} : {value}',
}) => {
  const sorted = useMemo(() => (data || []).filter(d => typeof d.time === 'number' && typeof d.value === 'number').slice().sort((a, b) => a.time - b.time), [data]);

  if (!sorted.length) return <div style={{ width: '100%', height }} />;

  const padding = { left: 64, right: 12, top: 8, bottom: 22 };
  const vw = 1000;
  const vh = height;
  const innerW = vw - padding.left - padding.right;
  const innerH = vh - padding.top - padding.bottom;

  const tMin = sorted[0].time;
  const tMax = sorted[sorted.length - 1].time === tMin ? tMin + 1 : sorted[sorted.length - 1].time;

  const xScale = linearScale([tMin, tMax], [0, innerW]);
  const yScale = linearScale([minY, maxY], [innerH, 0]);

  // x ticks aligned to hour boundaries
  const startDate = new Date(tMin * 1000);
  startDate.setMinutes(0, 0, 0);
  let start = Math.floor(startDate.getTime() / 1000);
  const xTicks: number[] = [];
  for (let t = start; t <= tMax; t += xStep) xTicks.push(t);

  // y ticks using yStep
  const yTicks: number[] = [];
  const yStart = Math.ceil(minY / yStep) * yStep;
  for (let v = yStart; v <= maxY; v += yStep) yTicks.push(v);

  const pathD = sorted.map((p, i) => `${i === 0 ? 'M' : 'L'} ${padding.left + Math.round(xScale(p.time))} ${padding.top + Math.round(yScale(p.value))}`).join(' ');

  // points positions for interactions
  const points = sorted.map(p => ({
    time: p.time,
    value: p.value,
    x: padding.left + Math.round(xScale(p.time)),
    y: padding.top + Math.round(yScale(p.value)),
  }));

  const [hoverIndex, setHoverIndex] = useState<number | null>(null);
  const containerRef = useRef<HTMLDivElement | null>(null);
  const [tooltip, setTooltip] = useState<{visible: boolean; x: number; y: number; text: string}>({visible:false,x:0,y:0,text:''});

  const formatValue = (v: number) => (Math.abs(v - Math.round(v)) < 1e-6 ? String(Math.round(v)) : v.toFixed(1));
  const buildLabel = (tpl: string, t: number, v: number) => tpl.replace('{time}', formatTime(t)).replace('{value}', formatValue(v));

  return (
    <div ref={containerRef} style={{ position: 'relative', width: '100%', height }}>
      <svg viewBox={`0 0 ${vw} ${vh}`} width="100%" height="100%" preserveAspectRatio="none">
        {/* horizontal grid + Y labels */}
        {yTicks.map((yt, idx) => {
          const y = padding.top + Math.round(yScale(yt));
          return (
            <g key={idx}>
                      <line x1={padding.left} x2={vw - padding.right} y1={y} y2={y} stroke="#ddd" strokeWidth={1} />
                      <text x={padding.left - 16} y={y + 4} textAnchor="end" fontSize={14} fontWeight={700} className={labelClass ? `${labelClass} fill-current` : undefined} fill={labelClass ? undefined : '#333'}>{Math.round(yt)}</text>
            </g>
          );
        })}

        {/* vertical x ticks and labels */}
        {xTicks.map((xt, idx) => {
          const x = padding.left + Math.round(xScale(xt));
          return (
            <g key={idx}>
              <line x1={x} x2={x} y1={padding.top} y2={vh - padding.bottom} stroke="#ddd" strokeWidth={1} strokeDasharray="4 4" />
              <text x={x} y={vh - 4} textAnchor="middle" fontSize={14} fontWeight={700} className={labelClass ? `${labelClass} fill-current` : undefined} fill={labelClass ? undefined : '#333'}>{formatTime(xt)}</text>
            </g>
          );
        })}

        {/* data path */}
        <path d={pathD} fill="none" stroke={lineClass ? 'currentColor' : '#1976d2'} strokeWidth={2} strokeLinejoin="round" strokeLinecap="round" className={lineClass ? `${lineClass} stroke-current` : undefined} />

        {/* interactive points */}
        {points.map((pt, idx) => (
          <g key={idx}>
            <circle
              cx={pt.x}
              cy={pt.y}
              r={hoverIndex === idx ? 5 : 2}
              fill={lineClass ? 'currentColor' : '#1976d2'}
              className={lineClass ? `${lineClass} fill-current` : undefined}
              onMouseEnter={(e) => {
                setHoverIndex(idx);
                const text = buildLabel(labelTemplate, pt.time, pt.value);
                const rect = containerRef.current?.getBoundingClientRect();
                if (rect) {
                  const svgWidth = rect.width;
                  const svgHeight = rect.height;
                  const approxCharWidth = 7;
                  const paddingBox = 8;
                  const textWidth = Math.max(40, text.length * approxCharWidth);
                  const boxWidth = textWidth + paddingBox;
                  const svgPixelX = (pt.x / vw) * svgWidth;
                  const svgPixelY = (pt.y / vh) * svgHeight;
                  let left = svgPixelX - boxWidth / 2;
                  if (left < 0) left = 0;
                  if (left > svgWidth - boxWidth) left = svgWidth - boxWidth;
                  const boxHeight = 24;
                  let top = svgPixelY - boxHeight - 8;
                  if (top < 0) top = 0;
                  setTooltip({ visible: true, x: Math.round(left), y: Math.round(top), text });
                } else {
                  setTooltip({ visible: true, x: pt.x + 8, y: pt.y - 28, text });
                }
              }}
              onMouseMove={() => {
                // recompute position in case container resized while hovering
                const rect = containerRef.current?.getBoundingClientRect();
                if (rect) {
                  const svgWidth = rect.width;
                  const svgHeight = rect.height;
                  const text = buildLabel(labelTemplate, pt.time, pt.value);
                  const approxCharWidth = 7;
                  const paddingBox = 8;
                  const textWidth = Math.max(40, text.length * approxCharWidth);
                  const boxWidth = textWidth + paddingBox;
                  const svgPixelX = (pt.x / vw) * svgWidth;
                  const svgPixelY = (pt.y / vh) * svgHeight;
                  let left = svgPixelX - boxWidth / 2;
                  if (left < 0) left = 0;
                  if (left > svgWidth - boxWidth) left = svgWidth - boxWidth;
                  const boxHeight = 24;
                  let top = svgPixelY - boxHeight - 8;
                  if (top < 0) top = 0;
                  setTooltip(s => ({ ...s, x: Math.round(left), y: Math.round(top) }));
                }
              }}
              onMouseLeave={() => { setHoverIndex(null); setTooltip({ visible: false, x: 0, y: 0, text: '' }); }}
            />
          </g>
        ))}
      </svg>
      {/* HTML tooltip that follows cursor */}
      {tooltip.visible ? (
        <div
          style={{
            position: 'absolute',
            left: tooltip.x,
            top: tooltip.y,
            pointerEvents: 'none',
            zIndex: 9999,
          }}
        >
          <div className={`${tooltipBgClass} text-white text-xs px-2 py-0.5 rounded shadow ${tooltipClass ?? ''}`}>
            <div style={{ textAlign: 'center', whiteSpace: 'nowrap' }}>{tooltip.text}</div>
          </div>
        </div>
      ) : null}
    </div>
  );
};

export default HistoryChart;
