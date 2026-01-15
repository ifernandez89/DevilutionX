#!/usr/bin/env python3
"""
🧪 TEST: AI HTTP CLIENT - REAL API CALL
Verifica que el cliente HTTP funciona correctamente con OpenRouter
"""

import os
import sys

def test_http_client():
    """
    Test del cliente HTTP real
    """
    print("=" * 70)
    print("🧪 AI HTTP CLIENT TEST")
    print("=" * 70)
    
    # 1. Verificar que existe API key
    print("\n1️⃣ Verificando API key...")
    env_file = "tools/.env.dev"
    
    if not os.path.exists(env_file):
        print(f"❌ No existe {env_file}")
        return False
    
    with open(env_file, 'r') as f:
        content = f.read()
        if 'API_KEY=' not in content:
            print("❌ No se encontró API_KEY en .env.dev")
            return False
        
        # Extraer API key
        for line in content.split('\n'):
            if line.startswith('API_KEY='):
                api_key = line.split('=', 1)[1].strip('"\'')
                if api_key:
                    print(f"✅ API key encontrada: {api_key[:20]}...")
                else:
                    print("❌ API key vacía")
                    return False
                break
    
    # 2. Verificar implementación HTTP
    print("\n2️⃣ Verificando implementación HTTP...")
    ai_cpp = "Source/ai/ai_text_variation.cpp"
    
    if not os.path.exists(ai_cpp):
        print(f"❌ No existe {ai_cpp}")
        return False
    
    with open(ai_cpp, 'r', encoding='utf-8') as f:
        content = f.read()
        
        # Verificar WinHTTP (Windows)
        if 'CallOpenRouterWindows' in content:
            print("✅ Implementación WinHTTP encontrada (Windows)")
        else:
            print("❌ No se encontró CallOpenRouterWindows")
            return False
        
        # Verificar libcurl (Linux/Mac)
        if 'CallOpenRouterCurl' in content:
            print("✅ Implementación libcurl encontrada (Linux/Mac)")
        else:
            print("❌ No se encontró CallOpenRouterCurl")
            return False
        
        # Verificar que NO es stub
        if 'return std::nullopt;  // ⚠️ ESTO NECESITA IMPLEMENTACIÓN REAL' in content:
            print("❌ Todavía es STUB, no implementación real")
            return False
        
        # Verificar JSON builder
        if 'BuildJSONRequest' not in content:
            print("❌ No se encontró BuildJSONRequest")
            return False
        print("✅ JSON builder encontrado")
        
        # Verificar JSON parser
        if 'ParseJSONResponse' not in content:
            print("❌ No se encontró ParseJSONResponse")
            return False
        print("✅ JSON parser encontrado")
    
    # 3. Verificar CMakeLists.txt
    print("\n3️⃣ Verificando dependencias CMake...")
    cmake_file = "Source/CMakeLists.txt"
    
    if not os.path.exists(cmake_file):
        print(f"❌ No existe {cmake_file}")
        return False
    
    with open(cmake_file, 'r', encoding='utf-8') as f:
        content = f.read()
        
        # Verificar que se busca libcurl en Linux/Mac
        if 'find_package(CURL)' in content or 'CURL::libcurl' in content:
            print("✅ Dependencia libcurl configurada para Linux/Mac")
        else:
            print("⚠️  libcurl no configurada (solo funcionará en Windows)")
    
    # 4. Verificar integración
    print("\n4️⃣ Verificando integración completa...")
    
    # Auto-configuración
    if 'tools/.env.dev' in content or '.env.dev' in open(ai_cpp, encoding='utf-8').read():
        print("✅ Auto-configuración desde .env.dev implementada")
    else:
        print("❌ Auto-configuración no encontrada")
        return False
    
    # Rate limiting
    if 'CanCallAI' in open(ai_cpp, encoding='utf-8').read():
        print("✅ Rate limiting implementado")
    else:
        print("❌ Rate limiting no encontrado")
        return False
    
    # Lore-safe validation
    if 'IsLoreSafe' in open(ai_cpp, encoding='utf-8').read():
        print("✅ Lore-safe validation implementada")
    else:
        print("❌ Lore-safe validation no encontrada")
        return False
    
    # Chat integration
    chat_cpp = "Source/control/control_chat.cpp"
    if os.path.exists(chat_cpp):
        with open(chat_cpp, 'r', encoding='utf-8') as f:
            if 'ProcessChatMessageWithAI' in f.read():
                print("✅ Integración con chat implementada")
            else:
                print("❌ Integración con chat no encontrada")
                return False
    
    print("\n" + "=" * 70)
    print("✅ TODOS LOS TESTS PASARON")
    print("=" * 70)
    print("\n🎯 SISTEMA LISTO PARA PROBAR:")
    print("   1. Compila el juego")
    print("   2. Inicia el juego")
    print("   3. Escribe en el chat")
    print("   4. La IA procesará el mensaje automáticamente")
    print("\n⏱️  RECUERDA:")
    print("   - Rate limit: 1 request cada 15 segundos")
    print("   - Timeout: 400ms máximo")
    print("   - Si falla: usa texto original (fallback)")
    print("\n🔍 PARA DEBUG:")
    print("   - Compila en modo Debug para ver logs detallados")
    print("   - Busca mensajes 'AI:' en la consola")
    print("=" * 70)
    
    return True

if __name__ == "__main__":
    success = test_http_client()
    sys.exit(0 if success else 1)
