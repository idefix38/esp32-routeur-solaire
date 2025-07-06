import { Home, Wifi, Settings, Sun } from 'lucide-react'

export const routes = [
    { path: '/', name: 'Accueil', component: () => import('./pages/home'), icon: Home },
    { path: '/wifi', name: 'WiFi', component: () => import('./pages/wifi'), icon: Wifi },
    { path: '/mqtt', name: 'MQTT', component: () => import('./pages/mqtt'), icon: Settings },
    { path: '/solar', name: 'Solar', component: () => import('./pages/solar'), icon: Sun },
];