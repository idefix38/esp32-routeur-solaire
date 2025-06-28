import { createContext } from 'preact';
import { useContext, useState } from 'preact/hooks';

export type ToastType = 'success' | 'error';
export interface ToastState {
  message: string;
  type: ToastType;
}

interface ToastContextProps {
  toast: ToastState | null;
  setToast: (toast: ToastState | null) => void;
}

const ToastContext = createContext<ToastContextProps>({
  toast: null,
  setToast: () => {},
});

export const ToastProvider = ({ children }: { children: any }) => {
  const [toast, setToast] = useState<ToastState | null>(null);
  return (
    <ToastContext.Provider value={{ toast, setToast }}>
      {children}
    </ToastContext.Provider>
  );
};

export const useToast = () => useContext(ToastContext);
