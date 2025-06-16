import { useEffect, useState } from 'preact/hooks';
import { pagePros } from '../app';
import { Temperature } from '../component/temperature';

export default function HomePage( props : pagePros) {
  const [temperature, setTemperature] = useState<number | null>(null);

  const fetchTemperature = async () => {
    try {
      const response = await fetch('/getTemperature');
      if (!response.ok) {
        throw new Error('Erreur lors de la récupération de la température');
      }
      const data = await response.json();
      setTemperature(parseFloat(data.temperature));
    } catch (error) {
      console.error('Erreur:', error);
    }
  };

  useEffect(() => {
    fetchTemperature(); // Récupère la température au chargement de la page
    const interval = setInterval(fetchTemperature, 30000); // Met à jour toutes les 30 secondes
    return () => clearInterval(interval); // Nettoie l'intervalle lors du démontage du composant
  }, []);

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
          <Temperature value={temperature ?? 0} />
        </div>
        <h1 className="text-xl font-semibold">Page d'accueil</h1>
        <p className="mt-4 text-lg">
          Température actuelle : {temperature !== null ? `${temperature}°C` : 'Chargement...'}
        </p>
      </div>
    </div>
  );
}
