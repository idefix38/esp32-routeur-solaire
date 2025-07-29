// Contexte Toast pour afficher des notifications globales dans l'application
import { createContext } from 'preact';
import { useContext, useState } from 'preact/hooks';

// Types de toast possibles
export type ToastType = 'success' | 'error';

// Structure d'un toast
export interface ToastState {
  message: string; // Message à afficher
  type: ToastType; // Type de toast (success ou error)
}

// Interface du contexte Toast
interface ToastContextProps {
  toast: ToastState | null; // Toast courant (ou null si aucun)
  setToast: (toast: ToastState | null) => void; // Fonction pour afficher ou masquer un toast
}

// Création du contexte avec des valeurs par défaut
const ToastContext = createContext<ToastContextProps>({
  toast: null,
  setToast: () => {},
});

// Provider à placer en haut de l'arbre React/Preact
export const ToastProvider = ({ children }: { children: any }) => {
  const [toast, setToast] = useState<ToastState | null>(null); // État du toast courant
  return (
    <ToastContext.Provider value={{ toast, setToast }}>
      {children}
    </ToastContext.Provider>
  );
};

// Hook pour accéder facilement au contexte Toast
export const useToast = () => useContext(ToastContext);