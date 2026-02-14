import { createContext } from "preact";
import { useContext, useEffect, useState } from "preact/hooks";
import { useEsp32Api } from "../hooks/useEsp32Api";

/**
 *  Parmètres de configuration du wifi
 */
export type wifiConfig= {
    ssid: string;
    password: string;
}

/**
 *  Parmètres de configuration du mqtt
 */
export type mqttConfig= {
    server: string;
    port: number;
    username: string;
    password: string;
    topic: string;
}

/**
 *  Paramètres de configuration du shellyEm
 */
export type shellyEmConfig ={
    ip: string;
    channel: string;
}

/**
 *  Paramètres de configuration du chauffe eau
 */
export type boilerConfig= {
    mode: string; // Auto, On ou Off  Auto = Routeur solaire , On = Marche forcée
    temperature?: number; // Températue cible du chauffe eau
    periods?: period[]; // List of heating periods
    triacOpening?: number; // Pourcentage d'ouverture du triac en mode manuel (0-100)
}


export type solarConfig= {
    latitude : number;
    longitude : number; 
    timeZone : string;       
    sunRiseMinutes? : number;
    sunSetMinutes? : number;    
}

export type period= {    
    start: number; // Heure de début de la période en minutes    
    startSunrise?: boolean; // La période commence au lever du soleil
    startSunset?: boolean;  // La période commence au coucher du soleil
    end: number; // Heure de fin de la période en minutes
    endSunrise?: boolean; // La période se termine au lever du soleil
    endSunset?: boolean;  // La période se termine au coucher du soleil
    mode: 'auto' | 'on'; // Mode de la période : 'auto' pour suivre le soleil, 'on' pour marche forcée
}


/**
 *  Paramètres de configuration
 */
export type config ={
    wifi: wifiConfig;
    mqtt: mqttConfig;
    shellyEm: shellyEmConfig;
    boiler: boilerConfig;
    solar: solarConfig;
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

