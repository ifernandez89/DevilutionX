# Guía Maestra de Integración: Botón de Donaciones y Pagos PayPal

Este documento contiene la totalidad de los requisitos, credenciales, configuraciones, variables de entorno y archivos de código necesarios para integrar un sistema de donaciones / pagos PayPal en cualquier aplicación web (React, Next.js, Astro, Vanilla JS o Backend Node.js).

---

## 📋 1. Requisitos Previos de Cuenta PayPal

Para recibir donaciones y pagos por PayPal existen dos modalidades principales:

### 1.1. Cuenta PayPal Personal / PayPal.Me (Nivel Básico)
- **Uso**: Enlaces directos simples de donación.
- **Requisitos**:
  1. Cuenta activa en [PayPal](https://www.paypal.com).
  2. Configurar enlace público [PayPal.Me](https://www.paypal.me/) (ejemplo: `https://paypal.me/TuUsuario/5USD`).
  3. Correo electrónico verificado y cuenta bancaria / tarjeta asociada para retirar fondos.

### 1.2. Cuenta PayPal Business & Developer Portal (Nivel Profesional / SDK)
- **Uso**: Botones integrados interactivos (tarjetas de crédito/débito dentro del sitio, selección de divisas, confirmación en vivo y webhooks).
- **Requisitos**:
  1. Registrarse o migrar a **PayPal Business Account** (gratuito).
  2. Acceder al [PayPal Developer Dashboard](https://developer.paypal.com/dashboard/).
  3. Crear una **App** en la sección *REST API apps*:
     - Modo **Sandbox** (para desarrollo y pruebas sin dinero real).
     - Modo **Live** (para producción).
  4. Obtener las siguientes credenciales:
     - `Client ID` (Público: se usa en frontend).
     - `Client Secret` (Privado: NUNCA exponer en frontend, solo en backend/APIs).
  5. (Opcional) Configurar URL de **Webhook** para escuchar eventos como `PAYMENT.CAPTURE.COMPLETED`.

---

## 🔐 2. Variables de Entorno (`.env`)

Crea o añade al archivo `.env.local` / `.env` las siguientes claves:

```env
# ==========================================
# PAYPAL CONFIGURATION
# ==========================================

# Modo: 'sandbox' para pruebas, 'production' para lanzamiento
NEXT_PUBLIC_PAYPAL_ENV=sandbox

# Client ID público (Sandbox o Live según corresponda)
NEXT_PUBLIC_PAYPAL_CLIENT_ID=TU_PAYPAL_CLIENT_ID_AQUI

# Enlace directo PayPal.Me para fallback / donación rápida
NEXT_PUBLIC_PAYPAL_ME_URL=https://paypal.me/TuUsuario

# Moneda por defecto (USD, EUR, etc.)
NEXT_PUBLIC_PAYPAL_CURRENCY=USD

# ------------------------------------------
# SOLO BACKEND / SERVIDOR (NUNCA EXPONER AL FRONTEND)
# ------------------------------------------
PAYPAL_CLIENT_SECRET=TU_PAYPAL_SECRET_AQUI
PAYPAL_WEBHOOK_ID=TU_PAYPAL_WEBHOOK_ID_AQUI
```

---

## 📦 3. Dependencias Requeridas

Si utilizas un entorno **React / Next.js**, instala el SDK oficial:

```bash
# Con bun
bun add @paypal/react-paypal-js

# O con npm
npm install @paypal/react-paypal-js

# O con yarn / pnpm
pnpm add @paypal/react-paypal-js
```

Para entornos **HTML / Vanilla JS**:
```html
<script src="https://www.paypal.com/sdk/js?client-id=TU_CLIENT_ID&currency=USD&components=buttons"></script>
```

---

## 📁 4. Estructura de Archivos Recomendada

A continuación se detalla la estructura modular sugerida para tu proyecto:

```
src/ (o app/ / components/)
├── components/
│   ├── donation/
│   │   ├── DonationModal.tsx        # Modal o tarjeta con opciones de montos ($5, $10, $25, custom)
│   │   ├── PayPalButtonWrapper.tsx  # Componente SDK de PayPal oficial
│   │   └── PayPalQuickLink.tsx      # Botón simple directo a PayPal.Me (fallback)
├── lib/
│   └── paypal.ts                    # Helpers y tipados para llamadas al backend de PayPal
└── app/ (opcional - Next.js App Router API)
    └── api/
        └── paypal/
            ├── create-order/route.ts # Endpoint para crear órdenes seguras desde servidor
            └── webhook/route.ts      # Endpoint para validar notificaciones de pago IPN/Webhook
```

---

## 💻 5. Código de los Archivos de Integración

### 5.1. Componente de Donación Simple (Enlace Directo sin dependencias)
> Archivo: `components/donation/PayPalQuickLink.tsx`

```tsx
import React from 'react';

interface QuickDonateProps {
  username?: string;
  defaultAmount?: number;
  currency?: string;
  className?: string;
}

export const PayPalQuickLink: React.FC<QuickDonateProps> = ({
  username = 'TuUsuario',
  defaultAmount = 5,
  currency = 'USD',
  className = '',
}) => {
  const donateUrl = `https://www.paypal.com/donate/?business=${encodeURIComponent(
    username
  )}&currency_code=${currency}&amount=${defaultAmount}`;

  return (
    <a
      href={donateUrl}
      target="_blank"
      rel="noopener noreferrer"
      className={`inline-flex items-center justify-center gap-2 px-6 py-3 rounded-xl font-semibold text-white bg-[#0070BA] hover:bg-[#005ea6] transition-all shadow-lg hover:scale-105 active:scale-95 ${className}`}
      style={{
        backgroundColor: '#0070BA',
        color: '#FFFFFF',
        padding: '12px 24px',
        borderRadius: '12px',
        textDecoration: 'none',
        display: 'inline-flex',
        alignItems: 'center',
        gap: '8px',
        fontWeight: 'bold',
      }}
    >
      <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor">
        <path d="M7.076 21.337H2.47a.641.641 0 0 1-.633-.74L4.944.901C5.026.382 5.474 0 6 0h7.523c3.843 0 6.642.827 7.72 2.684.974 1.677.787 4.14-.54 6.78-1.558 3.097-4.402 5.093-8.236 5.093H9.28a.86.86 0 0 0-.85.73l-1.354 6.05z" />
      </svg>
      <span>Donar con PayPal (${defaultAmount} {currency})</span>
    </a>
  );
};
```

---

### 5.2. Componente Completo con SDK de PayPal (Smart Payment Buttons)
> Archivo: `components/donation/PayPalDonationBox.tsx`

```tsx
'use client';

import React, { useState } from 'react';
import { PayPalScriptProvider, PayPalButtons } from '@paypal/react-paypal-js';

const PRESET_AMOUNTS = [3, 5, 10, 25, 50];

export default function PayPalDonationBox() {
  const [amount, setAmount] = useState<number>(5);
  const [customAmount, setCustomAmount] = useState<string>('');
  const [success, setSuccess] = useState<boolean>(false);
  const [errorMsg, setErrorMsg] = useState<string | null>(null);

  const clientId = process.env.NEXT_PUBLIC_PAYPAL_CLIENT_ID || 'test';
  const currency = process.env.NEXT_PUBLIC_PAYPAL_CURRENCY || 'USD';

  const effectiveAmount = customAmount ? parseFloat(customAmount) || 1 : amount;

  return (
    <div style={{
      maxWidth: '440px',
      margin: '0 auto',
      padding: '24px',
      borderRadius: '16px',
      background: 'rgba(255, 255, 255, 0.05)',
      border: '1px solid rgba(255, 255, 255, 0.1)',
      backdropFilter: 'blur(10px)',
      color: '#fff',
      fontFamily: 'sans-serif'
    }}>
      <h3 style={{ fontSize: '1.4rem', marginBottom: '8px', textAlign: 'center' }}>
        ☕ Apoya el Proyecto
      </h3>
      <p style={{ fontSize: '0.9rem', color: '#aaa', textAlign: 'center', marginBottom: '20px' }}>
        Selecciona un monto de donación para ayudar a mantener los servidores y el desarrollo.
      </p>

      {/* Selector de montos */}
      <div style={{ display: 'flex', gap: '8px', justifyContent: 'center', marginBottom: '16px', flexWrap: 'wrap' }}>
        {PRESET_AMOUNTS.map((val) => (
          <button
            key={val}
            onClick={() => { setAmount(val); setCustomAmount(''); }}
            style={{
              padding: '8px 16px',
              borderRadius: '8px',
              border: amount === val && !customAmount ? '2px solid #0070ba' : '1px solid rgba(255,255,255,0.2)',
              background: amount === val && !customAmount ? '#0070ba' : 'rgba(255,255,255,0.08)',
              color: '#fff',
              cursor: 'pointer',
              fontWeight: 'bold',
            }}
          >
            ${val}
          </button>
        ))}
      </div>

      {/* Input de monto personalizado */}
      <div style={{ marginBottom: '20px' }}>
        <input
          type="number"
          min="1"
          placeholder="Otro monto personalizado ($ USD)"
          value={customAmount}
          onChange={(e) => setCustomAmount(e.target.value)}
          style={{
            width: '100%',
            padding: '10px 14px',
            borderRadius: '8px',
            border: '1px solid rgba(255,255,255,0.2)',
            background: 'rgba(0,0,0,0.3)',
            color: '#fff',
            boxSizing: 'border-box',
          }}
        />
      </div>

      {/* Mensajes de éxito o error */}
      {success && (
        <div style={{ padding: '12px', background: '#1b5e20', borderRadius: '8px', marginBottom: '16px', textAlign: 'center' }}>
          🎉 ¡Muchísimas gracias por tu apoyo y donación!
        </div>
      )}

      {errorMsg && (
        <div style={{ padding: '12px', background: '#b71c1c', borderRadius: '8px', marginBottom: '16px', textAlign: 'center' }}>
          ⚠️ {errorMsg}
        </div>
      )}

      {/* Botones de PayPal SDK */}
      {!success && (
        <PayPalScriptProvider
          options={{
            clientId: clientId,
            currency: currency,
            intent: 'capture',
          }}
        >
          <PayPalButtons
            style={{ layout: 'vertical', shape: 'rect', label: 'donate' }}
            forceReRender={[effectiveAmount, currency]}
            createOrder={(data, actions) => {
              return actions.order.create({
                purchase_units: [
                  {
                    description: 'Donación al proyecto',
                    amount: {
                      currency_code: currency,
                      value: effectiveAmount.toFixed(2),
                    },
                  },
                ],
                application_context: {
                  shipping_preference: 'NO_SHIPPING', // No solicita dirección física
                },
              });
            }}
            onApprove={async (data, actions) => {
              if (actions.order) {
                const details = await actions.order.capture();
                console.log('Pago completado con éxito:', details);
                setSuccess(true);
                setErrorMsg(null);
              }
            }}
            onError={(err) => {
              console.error('Error en PayPal:', err);
              setErrorMsg('Hubo un problema al procesar la donación. Intenta nuevamente.');
            }}
          />
        </PayPalScriptProvider>
      )}
    </div>
  );
}
```

---

### 5.3. Endpoint de Verificación Backend (Opcional pero recomendado para seguridad)
> Archivo: `app/api/paypal/create-order/route.ts` (Next.js App Router)

```typescript
import { NextResponse } from 'next/server';

export async function POST(req: Request) {
  try {
    const { amount, currency = 'USD' } = await req.json();

    const auth = Buffer.from(
      `${process.env.NEXT_PUBLIC_PAYPAL_CLIENT_ID}:${process.env.PAYPAL_CLIENT_SECRET}`
    ).toString('base64');

    const baseUrl =
      process.env.NEXT_PUBLIC_PAYPAL_ENV === 'production'
        ? 'https://api-m.paypal.com'
        : 'https://api-m.sandbox.paypal.com';

    const response = await fetch(`${baseUrl}/v2/checkout/orders`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        Authorization: `Basic ${auth}`,
      },
      body: JSON.stringify({
        intent: 'CAPTURE',
        purchase_units: [
          {
            amount: {
              currency_code: currency,
              value: Number(amount).toFixed(2),
            },
          },
        ],
        application_context: {
          shipping_preference: 'NO_SHIPPING',
        },
      }),
    });

    const order = await response.json();
    return NextResponse.json(order);
  } catch (error) {
    return NextResponse.json({ error: 'Error al crear orden PayPal' }, { status: 500 });
  }
}
```

---

## ✅ 6. Checklist de Validación antes del Lanzamiento

- [ ] **1. Prueba en Sandbox**:
  - Crear cuenta compradora ficticia en *Developer Dashboard > Sandbox Accounts*.
  - Probar flujo de pago completo con saldo ficticio o tarjeta de test.
- [ ] **2. Cambio de Credenciales a Live**:
  - Cambiar `NEXT_PUBLIC_PAYPAL_ENV=production`.
  - Reemplazar `NEXT_PUBLIC_PAYPAL_CLIENT_ID` con el Client ID de Producción.
  - Reemplazar `PAYPAL_CLIENT_SECRET` con el Secret de Producción.
- [ ] **3. Envío Sin Dirección de Envío (`NO_SHIPPING`)**:
  - Asegurado en la configuración para que el donante no tenga que ingresar dirección física de despacho.
- [ ] **4. Responsive & Mobile Friendly**:
  - Botón accesible con área de toque mínima de 44px.
  - Formato adaptable a teléfonos y escritorios.
- [ ] **5. Cumplimiento & Privacidad**:
  - Agregar breve leyenda de aviso legal o política de donaciones voluntarias según las leyes locales.
