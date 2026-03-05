/** @type {import('tailwindcss').Config} */
module.exports = {
  content: ['./src/renderer/**/*.{html,tsx,ts}'],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        bg: '#0A0A0F',
        surface: '#12121A',
        elevated: '#1A1A25',
        border: 'rgba(255,255,255,0.08)',
        primary: '#EDEDEF',
        secondary: '#888893',
        accent: '#8B8CF8',
        success: '#34D399',
        warning: '#FB923C',
        destructive: '#EF4444',
      },
      borderRadius: {
        card: '8px',
        input: '6px',
        sm: '4px',
      },
      fontFamily: {
        sans: ['Inter', 'system-ui', 'sans-serif'],
      },
      boxShadow: {
        subtle: '0 1px 2px rgba(0,0,0,0.3)',
      },
    },
  },
  plugins: [],
};
