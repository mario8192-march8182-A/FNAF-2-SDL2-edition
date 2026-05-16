# Suporte Xbox e Epic Games Store

## Xbox
- Teste no seu **Dev Kit** por conta própria.
- Certifique-se de que sua empresa possui acesso oficial ao SDK da Microsoft.
- Não distribua builds não autorizados — isso pode causar problemas legais e técnicos.
- Estrutura:
  - `Assets/` → sprites, sons e UI
  - `Source/` → código em C++ com SDL2
  - `Platform/Xbox/` → ajustes específicos para Xbox

## Epic Games Store
- Compile a versão PC (Windows/Linux) normalmente.
- Empacote executável e assets em formato aceito pela Epic.
- (Opcional) Integre **Epic Online Services** para conquistas, login e multiplayer.
- Publique via painel da Epic Games Store Developer.

⚠️ Aviso: Este projeto é experimental e não possui suporte oficial da Microsoft ou Epic.  
Use apenas em ambientes de desenvolvimento autorizados.
