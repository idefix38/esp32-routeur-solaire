// Hook personnalisé pour centraliser les appels API vers l'ESP32
// Gère les routes définies dans WebServerManager::setupApiRoutes()
// Retourne un objet { data, success } pour chaque appel

import { useState, useCallback } from 'preact/hooks';

// Types des routes API disponibles
export type ApiRoute = '/saveWifiSettings' | '/saveMqttSettings' | '/getData' | '/saveSolarSettings' | '/saveBoilerSettings' | '/getConfig' | '/reboot' | '/api/update/check' | '/api/update/start';

// Structure de retour du callApi
interface ApiResult {
    data: any;      // Données retournées par l'API (ou null en cas d'erreur)
    success: boolean; // true si la requête a réussi, false sinon
}

// Structure retournée par le hook
interface UseEsp32ApiResult {
    loading: boolean; // Indique si une requête est en cours
    callApi: (
        route: ApiRoute,
        options?: RequestInit
    ) => Promise<ApiResult>;
}

// Récupère l'URL de base de l'API depuis les variables d'environnement (Vite)


// Hook principal
export function useEsp32Api(): UseEsp32ApiResult {
    const [loading, setLoading] = useState(false);

    // Fonction pour appeler une route API
    const callApi = useCallback(
        async (route: ApiRoute, options?: RequestInit): Promise<ApiResult> => {
            setLoading(true);
            try {
                // Si API_BASE_URL est défini (mode dev), on préfixe la route                
                const response = await fetch(route, options);
                // On tente de parser la réponse JSON
                const data = await response.json().catch(() => ({}));
                if (!response.ok) {
                    // Si la réponse n'est pas OK, on retourne success: false
                    return { data, success: false };
                }
                // Succès
                return { data, success: true };
            } catch (err) {
                // En cas d'erreur réseau ou autre
                return { data: null, success: false };
            } finally {
                setLoading(false);
            }
        },
        []
    );

    return { loading, callApi };
}
