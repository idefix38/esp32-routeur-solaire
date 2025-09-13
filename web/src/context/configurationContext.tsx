import { createContext } from "preact";
import { useContext, useEffect, useState } from "preact/hooks";
import { useEsp32Api } from "../hooks/useEsp32Api";

/**
 *  Parmètres de configuration du wifi
 */
export interface wifiConfig {
    ssid: string;
    password: string;
}

/**
 *  Parmètres de configuration du mqtt
 */
export interface mqttConfig {
    server: string;
    port: number;
    username: string;
    password: string;
    topic: string;
}

/**
 *  Paramètres de configuration du shellyEm
 */
export interface shellyEmConfig {
    ip: string;
    channel: string;
}

/**
 *  Paramètres de configuration du chauffe eau
 */
export interface boilerConfig {
    mode: string; // Auto, On ou Off  Auto = Routeur solaire , On = Marche forcée
    temperature?: number; // Températue cible du chauffe eau
}


/**
 *  Paramètres de configuration
 */
export interface config {
    wifi: wifiConfig;
    mqtt: mqttConfig;
    shellyEm: shellyEmConfig;
    boiler: boilerConfig;
}


interface ConfigContextProps {
    value: config | null;
    setConfig: (c: config | null) => void;
}


// Création du contexte avec des valeurs par défaut
const ConfigContext = createContext<ConfigContextProps>({
    value: null,
    setConfig: () => { },
});

// Provider à placer en haut de l'arbre React/Preact
export const ConfigProvider = ({ children }: { children: any }) => {
    const [value, setConfig] = useState<config | null>(null);
    const apiEsp32 =  useEsp32Api();

    // Charge la config
    useEffect(() => {
        (async () => {
            const result = await apiEsp32.callApi('/getConfig');
            console.log("getConfig du ConfigProvider", result.data)
            if (result.success && result.data) {
                setConfig(result.data);
            }
        })();
    }, []);


    return (
        <ConfigContext.Provider value= {{ value, setConfig }}>
             { children } 
        </ConfigContext.Provider>
  );
};

// Hook pour accéder facilement au contexte Config
export const useConfig = () => useContext(ConfigContext);

