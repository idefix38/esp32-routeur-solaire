import { Disclosure, DisclosureButton, DisclosurePanel, Popover } from '@headlessui/react'
import { Menu, X } from 'lucide-react'
import { useLocation } from 'preact-iso';
import { routes } from '../routes';
import styles from './navBar.module.scss';
import { CirclePower } from 'lucide-react';
import { useEsp32Api } from '../hooks/useEsp32Api';
import { useToast } from '../context/ToastContext';

function classNames(...classes: any[]) {
  return classes.filter(Boolean).join(' ')
}

export const Navbar = () => {
  const { path } = useLocation();
  const { setToast } = useToast();  
  const {callApi, loading} = useEsp32Api();


  return (
    <Disclosure as="nav" className={styles.navbar}>
      <div className="mx-auto max-w-7xl px-2 sm:px-6 lg:px-8">
        <div className="relative flex h-16 items-center justify-between">
          <div className="absolute inset-y-0 left-0 flex items-center sm:hidden">
            {/* Mobile menu button */}
            <DisclosureButton className="group relative inline-flex items-center justify-center rounded-md p-2 text-gray-400 hover:bg-gray-700 hover:text-white focus:outline-none focus:ring-2 focus:ring-inset focus:ring-white">
              <span className="absolute -inset-0.5" />
              <span className="sr-only">Open main menu</span>
              <Menu aria-hidden="true" className="block size-6 group-data-[open]:hidden" />
              <X aria-hidden="true" className="hidden size-6 group-data-[open]:block" />
            </DisclosureButton>
          </div>
          {/* Logo: centered on mobile, left on desktop */}
          <div className="absolute left-1/2 transform -translate-x-1/2 sm:hidden">
            <a href="/">
              <img src="/solar.svg" alt="Solar logo" className="h-12 w-auto" />
            </a>
          </div>
          <div className="flex flex-1 items-center justify-center sm:items-stretch sm:justify-start">
            {/* Desktop logo on the left (hidden on mobile) */}
            <a href="/" className="hidden sm:flex items-center mr-4">
              <img src="/solar.svg" alt="Solar logo" className="h-12 w-auto" />
            </a>
            <div className="hidden sm:ml-6 sm:block">
              <div className="flex space-x-4">
                {routes.map((item) => (
                  <a
                    key={item.name}
                    href={item.path}
                    aria-current={item.path == path ? 'page' : undefined}
                    className={classNames(
                      item.path == path
                        ? 'bg-indigo-500 text-white'
                        : 'text-gray-300 hover:bg-indigo-800 hover:text-white',
                      'rounded-md px-3 py-2 text-sm font-medium flex items-center space-x-2'
                    )}
                  >
                    <item.icon className="size-5" aria-hidden="true" />
                    <span>&nbsp;{item.name}</span>
                  </a>
                ))}
              </div>
            </div>
          </div>          
          <div className="absolute inset-y-0 right-0 flex items-center pr-2">
            <Popover className="relative">
              {({ close }) => (
                <>
                  <Popover.Button className="h-full flex items-center rounded-md p-0 text-sm font-medium text-gray-300 hover:bg-indigo-800 hover:text-white focus:outline-none">
                    <CirclePower className="size-6" aria-hidden="true" />
                  </Popover.Button>
                  <Popover.Panel className="absolute right-0 top-full mt-2 w-48 rounded-md bg-white shadow-lg ring-1 ring-black ring-opacity-5 focus:outline-none z-10">
                    <div className="py-1">
                      <button
                        className="w-full text-left px-4 py-2 text-sm text-gray-700 hover:bg-gray-100 disabled:opacity-50"
                        onClick={async () => {
                          const result = await callApi("/reboot", { method: "POST" });
                          if (result.success) {
                            setToast({ message: 'Redemarrage en cours...', type: 'success' });
                          } else {
                            setToast({ message: 'Erreur impossible de redémarrer le routeur', type: 'error' });
                          }
                          close(); // <-- Add this line to close the popover
                        }}
                        disabled={loading}
                      >
                        Redémarrer l'ESP32
                      </button>
                    </div>
                  </Popover.Panel>
                </>
              )}
            </Popover>
          </div>
        </div>
      </div>

      <DisclosurePanel className="sm:hidden">
        <div className="space-y-1 px-2 pb-3 pt-2">
          {routes.map((item) => (
            <DisclosureButton
              key={item.name}
              as="a"
              href={item.path}
              aria-current={item.path == path ? 'page' : undefined}
              className={classNames(
                item.path == path
                  ? 'bg-indigo-500 text-white'
                  : 'text-gray-300 hover:bg-indigo-500 hover:text-white',
                'block rounded-md px-3 py-2 text-base font-medium space-x-2'
              )}
            >
              <item.icon className="size-5" aria-hidden="true" />
              <span>  {item.name}</span>
            </DisclosureButton>
          ))}
        </div>
      </DisclosurePanel>
    </Disclosure>
  );
};