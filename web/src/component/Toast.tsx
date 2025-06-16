import { useEffect, useState } from 'preact/hooks';

interface ToastProps {
  message: string;
  type: 'success' | 'error';
  onClose: () => void;
}

export const Toast = ({ message, type, onClose }: ToastProps) => {
  const [visible, setVisible] = useState(true);

  useEffect(() => {
    const timer = setTimeout(() => {
      setVisible(false);
      setTimeout(onClose, 1000); // Appelle onClose après 1 seconde (durée du fondu)
    }, 3000); // 3 secondes avant de commencer le fondu

    return () => clearTimeout(timer);
  }, [onClose]);

  return (
    <div
      className={`fixed top-4 right-4 z-50 p-4 rounded shadow-lg text-white transition-opacity duration-1000 ${
        visible ? 'opacity-100' : 'opacity-0'
      } ${type === 'success' ? 'bg-green-500' : 'bg-red-500'}`}
    >
      <div className="flex items-center justify-between">
        <span>{message}</span>
        <button
          onClick={onClose}
          className="ml-4 text-white hover:text-gray-200 focus:outline-none"
        >
          ✕
        </button>
      </div>
    </div>
  );
};
