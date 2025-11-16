import { useState, useEffect, useRef } from 'preact/hooks';

// Structure des données reçues via WebSocket
interface WebSocketData {
    temperature?: number;
    sunrise?: string;
    sunset?: string;
    triacOpeningPercentage?: number;
    temperatureReached?: boolean;
    currentFirmwareVersion?: string;
    newFirmwareVersion?: string;
}

// Énumération pour le statut de la connexion
export enum ConnectionStatus {
    Connecting = 'Connecting',
    Open = 'Open',
    Closing = 'Closing',
    Closed = 'Closed',
}

/**
 * Hook pour gérer la connexion WebSocket avec l'ESP32.
 * @returns Un objet contenant les dernières données reçues et le statut de la connexion.
 */
export function useEsp32WebSocket() {
    const [data, setData] = useState<WebSocketData>({ temperature: undefined, triacOpeningPercentage: undefined, temperatureReached: false, currentFirmwareVersion: undefined, newFirmwareVersion: undefined });
    const [status, setStatus] = useState<ConnectionStatus>(ConnectionStatus.Closed);
    const ws = useRef<WebSocket | null>(null);

    useEffect(() => {
        // Ne s'exécute que côté client
        if (typeof window === 'undefined') {
            return;
        }

        const connect = () => {
            // Construit l'URL WebSocket à partir de l'hôte actuel
            const url = `ws://${window.location.host}/ws`;
            ws.current = new WebSocket(url);
            setStatus(ConnectionStatus.Connecting);

            ws.current.onopen = () => {
                console.log('WebSocket connection established');
                setStatus(ConnectionStatus.Open);
            };

            ws.current.onmessage = (event) => {
                try {
                    const message = JSON.parse(event.data);
                    // Met à jour l'état avec les nouvelles données
                    setData(message);
                } catch (error) {
                    console.error('Failed to parse WebSocket message:', error);
                }
            };

            ws.current.onclose = () => {
                console.log('WebSocket connection closed. Reconnecting...');
                setStatus(ConnectionStatus.Closed);
                // Tente de se reconnecter après 3 secondes
                setTimeout(connect, 3000);
            };

            ws.current.onerror = (error) => {
                console.error('WebSocket error:', error);
                ws.current?.close(); // Déclenchera onclose et la tentative de reconnexion
            };
        };

        connect();

        // Fonction de nettoyage pour fermer la connexion lors du démontage du composant
        return () => {
            if (ws.current) {
                // Empêche la logique de reconnexion lors du démontage manuel
                ws.current.onclose = () => {
                    console.log('WebSocket connection closed on component unmount.');
                };
                ws.current.close();
            }
        };
    }, []); // Le tableau de dépendances vide assure que l'effet ne s'exécute qu'une seule fois

    return { data, status };
}
