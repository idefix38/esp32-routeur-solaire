import { pagePros } from '../app';
import { Temperature } from '../component/temperature';

export default function HomePage( props : pagePros) {
  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
          <Temperature value={22.5} />
        </div>
      </div>
    </div>
  );
}
