import { Disclosure, DisclosureButton, DisclosurePanel } from '@headlessui/react'
import { Menu, X } from 'lucide-react'
import { useLocation } from 'preact-iso';
import { routes } from '../routes';
import styles from './navBar.module.scss';

function classNames(...classes: any[]) {
  return classes.filter(Boolean).join(' ')
}

export const Navbar = () => {
  const { path } = useLocation();
  console.debug("Path: " + path);

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
          <div className="flex flex-1 items-center justify-center sm:items-stretch sm:justify-start">
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