import React, { useMemo } from 'react';
import styles from './dualRangeSlider.module.scss';
import { LucideProps, Minus, Plus } from 'lucide-react';
import { formatMinuteToTime } from '../helper/time';

export interface DualRangeSliderOnChangeData {
  start: number;
  end: number;
  isSunrise: boolean;
  isSunset: boolean;
}

interface DualRangeSliderProps {
  min?: number;
  max?: number;
  step?: number;
  color?: string;
  sunrise?: number;
  sunset?: number;
  startValue?: number;
  endValue?: number;
  onChange?: (values: DualRangeSliderOnChangeData) => void;
  sunriseIcon?: React.ComponentType<LucideProps>;
  sunsetIcon?: React.ComponentType<LucideProps>;
}

const DualRangeSlider: React.FC<DualRangeSliderProps> = ({
  min = 0,
  max = 1440, // 24 * 60
  step = 1,
  color = '#FFA500', // Default to orange
  sunrise,
  sunset,
  startValue = min,
  endValue = max,
  onChange,
  sunriseIcon: SunriseIcon,
  sunsetIcon: SunsetIcon,
}) => {

  const triggerOnChange = (start: number, end: number) => {
    if (onChange) {
      onChange({
        start,
        end,
        isSunrise: start === sunrise,
        isSunset: end === sunset,
      });
    }
  };

  const handleStartChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = Math.min(Number((e.target as HTMLInputElement).value), endValue);
    triggerOnChange(value, endValue);
  };

  const handleEndChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const value = Math.max(Number((e.target as HTMLInputElement).value), startValue);
    triggerOnChange(startValue, value);
  };

  const handleDecrement = (setter: 'start' | 'end') => {
    if (setter === 'start') {
      triggerOnChange(Math.max(startValue - 1, min), endValue);
    } else {
      triggerOnChange(startValue, Math.max(endValue - 1, startValue));
    }
  };

  const handleIncrement = (setter: 'start' | 'end') => {
    if (setter === 'start') {
      triggerOnChange(Math.min(startValue + 1, endValue), endValue);
    } else {
      triggerOnChange(startValue, Math.min(endValue + 1, max));
    }
  };

  const startPercentage = useMemo(() => ((startValue - min) / (max - min)) * 100, [startValue, min, max]);
  const endPercentage = useMemo(() => ((endValue - min) / (max - min)) * 100, [endValue, min, max]);

  
  const handleSunriseClick = () => {
    if (sunrise) {
      triggerOnChange(sunrise, endValue);
    }
  };

  const handleSunsetClick = () => {
    if (sunset) {
      triggerOnChange(startValue, sunset);
    }
  };

  const hourMarkers = useMemo(() => {
    const markers = [];
    for (let i = 0; i <= 24; i += 2) {
      const percentage = (i * 60 / (max - min)) * 100;
      markers.push(
        <div key={i} className={styles.hourMarker} style={{ left: `${percentage}%` }}>
          <span>{i}h</span>
        </div>
      );
    }
    return markers;
  }, [min, max]);

  return (
    <div className={styles.sliderContainer}>
        <div className={styles.iconsContainer}>
            {SunriseIcon && sunrise && (
                <div
                    className={styles.iconMarkerWrapper}
                    style={{ left: `${((sunrise - min) / (max - min)) * 100}%` }}
                    onClick={handleSunriseClick}
                    title={`Heure de lever du soleil - ${formatMinuteToTime(sunrise)}`}
                >
                    <SunriseIcon className={styles.iconMarker} />
                </div>
            )}
            {SunsetIcon && sunset && (
                <div
                    className={styles.iconMarkerWrapper}
                    style={{ left: `${((sunset - min) / (max - min)) * 100}%` }}
                    onClick={handleSunsetClick}
                    title={`Heure de coucher du soleil - ${formatMinuteToTime(sunset)}`}
                >
                    <SunsetIcon className={styles.iconMarker} />
                </div>
            )}
        </div>

        <div className={styles.hourMarkersContainer}>{hourMarkers}</div>

        <div className={styles.trackContainer}>
            <div
            className={styles.track}
            style={{ background: `linear-gradient(to right, #ddd ${startPercentage}%, ${color} ${startPercentage}%, ${color} ${endPercentage}%, #ddd ${endPercentage}%)` }}
            ></div>
            <input
            type="range"
            min={min}
            max={max}
            step={step}
            value={startValue}
            onChange={handleStartChange}
            className={styles.slider}
            style={{ '--thumb-color': color } as React.CSSProperties}
            />
            <input
            type="range"
            min={min}
            max={max}
            step={step}
            value={endValue}
            onChange={handleEndChange}
            className={styles.slider}
            style={{ '--thumb-color': color } as React.CSSProperties}
            />
        </div>       
        <div className={styles.bottomControls}>
            <div className={styles.buttonGroup}>
                <button type="button" onClick={() => handleDecrement('start')} className={styles.actionButton}><Minus size={16}/></button>
                <button type="button" onClick={() => handleIncrement('start')} className={styles.actionButton}><Plus size={16}/></button>
            </div>
            <div className={styles.buttonGroup}>
                <button type="button" onClick={() => handleDecrement('end')} className={styles.actionButton}><Minus size={16}/></button>
                <button type="button" onClick={() => handleIncrement('end')} className={styles.actionButton}><Plus size={16}/></button>
            </div>
        </div>
    </div>
  );
};

export default DualRangeSlider;
