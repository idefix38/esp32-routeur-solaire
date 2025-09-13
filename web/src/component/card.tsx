import React from 'react';
import { LucideProps, CheckCircle2 } from 'lucide-react';

interface CardProps {
  value: string | null;
  label: string;
  Icon: React.ComponentType<LucideProps>;
  showCheck?: boolean;
}

export const Card = ({ value, label, Icon, showCheck = false }: CardProps) => {
  return (
    <div className="overflow-hidden rounded-lg bg-gray-100 shadow">
      <div className="p-5">
        <div className="flex items-center">
          <div className="flex-shrink-0">
            <Icon className="h-8 w-8 text-indigo-600" />
          </div>
          <div className="ml-5 w-0 flex-1">
            <dl>
              <dt className="text-sm font-medium text-gray-500 truncate">
                {label}
              </dt>
              <dd className="flex items-baseline">
                <div className="text-2xl font-semibold text-gray-900">
                  {value !== null ? `${value}` : '?'}
                </div>
              </dd>
            </dl>
          </div>
          {showCheck && (
            <div className="ml-4 flex-shrink-0">
              <CheckCircle2 className="h-6 w-6 text-green-700" />
            </div>
          )}
        </div>
      </div>
    </div>
  );
};