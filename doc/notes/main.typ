
#set text(
  lang: "en"
)

// allows figure to be split accross pages
#show figure: set block(breakable: true)

// set table title on top
#show figure.where(
  kind: table
): set figure.caption(position: top)


#align(center, text(32pt)[
  *Ecriture d'un UNIX like*
])

// define the table of content
#set heading(numbering: "1.")
#show outline.entry.where(level: 1):it => {
  v(12pt, weak: true)
  strong(it)
}
#outline(indent:4pt)
#pagebreak()

// content:

=  Parties manquantes
- Ecriture du secteur d'ammorcage
- Décider si le document doit être écris en anglais
- Documentation sur les périphériques avec les infos avérées

Il faudra déguager les section suivantes:
- architecture du kernel
- étude du harware armV6, périphériques

= System boot

= Context switching

#figure(
  table(
    columns: (auto, auto, 1fr),
    inset: 10pt,
    align: (center, center, left),
    table.header([*Offset*], [*Name*], [*Description*]),
    [31],   [N], [],
    [30],   [Z], [],
    [29],   [C], [],
    [28],   [V], [],
    [27:8], [_unused_], [],
    [7],    [I], [Disables IRQ interrupts when it is set],
    [6],    [F], [Disables FIQ interrupts when it is set],
    [5],    [T], [Thumb mode],
    [4:0],  [M], [cpu mode (see bellow)],
  ),
  caption: [CPSR - Current Program Status Register]
)

Le registre `CPSR` est sauvegardé dans le registre `SPSR` lors des changements de contexte.

#figure(
  table(
    columns: (auto, auto, auto, auto, auto),
    inset: 10pt,
    align: (left, center, left, left, center),
    table.header([*Mode*], [*Encoding*], [*Function*], [*Security State*], [*Privilege Level*]),
      [user],    [0x10], [Unprivileged mode in which most applications run],                         [Both],        [PL0],
      [fiq],     [0x11], [Entered on an FIQ interrupt exception],                                    [Both],        [PL1],
      [irq],     [0x12], [Entered on an IRQ interrupt exception],                                    [Both],        [PL1],
      [svc],     [0x13], [Entered on reset or when a Supervisor Call instruction (SVC) is executed], [Both],        [PL1],
      [monitor], [0x16], [Implemented with Security Extensions.],                                    [Secure only], [PL1],
      [abort],   [0x17], [Entered on a memory access exception],                                     [Both],        [PL1],
      [hyp],     [0x1a], [Implemented with Virtualization Extensions.],                              [Non-secure],  [PL2],
      [undef],   [0x1b], [Entered when an undefined instruction executed],                           [Both],        [PL1],
      [system],  [0x1f], [Privileged mode, sharing the register view with User mode],                [Both],        [PL1],
  ),
  caption: [CPU Modes]
)



= Process

== Gestion des IOS
- trois fichiers ouverts au départs: stdin, stdout, stderr

*sans fork:* multiplexer sur le mini uart.

== Notions de process parents/enfant
- créations de process enfants par le fork.

Dans notre cas, sans mémoire virtuelle, le fork est il faisable ?
- copie des descripteurs de fichiers: ok
- duplication des stacks: dangereux car des pointeurs de la stack vers elle même seraient invalidés......

=> conclusion: la mmu est necessaire pour le fork.


= Gestion de la mémoire virtuelle


== Principe de base
The MMU map virtual addresses used by the processes to physical addresses. A different mapping is used per process.

=== Activation de la mmu
- registre SCTLR (en mettant le bit M=1.)
- définition de la *translation table base* (TTBR0/TTBR1)
- flusher le TLB après chaque changement de contexte


*conséquence sur les processes:*
- Le pointeur de pile doit être une addresse virtuelle
- Le code executable doit également être mappé en mémoire

=== Gestion du code executable
Le code du process pause un problème particulier car ce dernier doit être également mappé en mémoire virtuelle.


*solution temporaire:* mapper le kernel dans la mémoire virtuelle, à la même adresse que sont addresse physique

== En pratique
Au départ utiliser un unique mapping identité et activer la mmu avec le mapping en question.

=== Fonctionement de la mmu sur arm
Il existe trois types de pages sur arm:
- Small pages : 4 KB (0x1000 octets) — utilisé dans la majorité des cas.
- Large pages : 64 KB (rarement utilisé).
- Sections    : 1 MB (utilisé pour des mappings simples, comme l'identité).

Les tables de pages sont hiérarchiques et divisées en deux niveau:

/ Tables de niveau 1: _(translation table base)_ \
  Mappe de larges plages d'adresses virtuelles à des blocs physiques (1 MB par entrée). Elle contient 4096 entrées (1 par MB d'espace virtuel).
/ Table de niveaux 2: \
  Utilisée pour diviser les 1 MB en pages de 4 KB. Chaque entrée de niveau 2 contient un mapping pour une page.

Un mapping initial identité peux donc se faire sans utiliser de table de niveaux 2, avec des sections de 1mb.

==== Format des tables de niveau 1
Chaque entrée (32 bits) peut représenter une section ou bien une _page table_.

===== Section entry
Une région de 1MB mappée directement (typiquement utile pour le mapping identité).
#figure(
  table(
    columns: (auto, auto, 1fr),
    inset: 10pt,
    align: (center, left, left),
    table.header([*Offset*], [*Name*], [*Description*]),
    [31:20], [], [Adresse physique des 12 bits les plus significatifs (base de 1 MB alignée).],
    [19:17], [], [_Should-Be-Zero_],
    [16],    [*`C`*], [  Active ou non le cache de données pour la section.],
    [15-12], [*`TEX`*], [ Type Extension
      - Dans la convention la plus répandue pour les sections, on utilise 3 bits TEX occupant les bits 14-12.
      - Le rôle du champ TEX est d'étendre la définition des attributs mémoire.
      - En combinaison avec le bit C (Cacheable) et le bit B (Bufferable, qui viendra plus bas), la triplette (TEX, C, B) permet de définir le type exact de mémoire.
      - Par exemple, on pourra avoir:
          - (`TEX = 000`, `C = 0`, `B = 0`) → _Mémoire fortement ordonnée (souvent pour les registres)._
          - (`TEX = 000`, `C = 0`, `B = 1`) → _Mémoire de type Device (non cacheable, adapté aux _périphériques).
          - (`TEX = 000`, `C = 1`, `B = 0`) → _Mémoire normale, non cacheable._
          - (`TEX = 000`, `C = 1`, `B = 1`) → _Mémoire normale, Write-Through._
          - (`TEX = 001`, `C = 1`, `B = 1`) → _Mémoire normale, Write-Back avec Write-Allocate._

      / Remarque: Les valeurs exactes disponibles et leur interprétation peuvent varier légèrement selon le processeur et la configuration de la MMU, mais le principe reste le même : le champ TEX (souvent défini sur 3 bits pour les sections) offre une granularité supplémentaire pour le choix des attributs mémoire.
    ],
    [11:10], [*`AP[1:0]`*], [ Accès utilisateur ou kernel (R/W).
      #table(
          columns: (auto, auto, auto, auto),
          stroke: none,
          table.header([*`AP[1]`*], [*`AP[0]`*], [*Mode Privilégié*], [*Mode Non-Privilégié*]),
          [0], [0],	[Lecture/Écriture], [Aucun accès],
          [0], [1],	[Lecture/Écriture], [Lecture uniquement],
          [1], [0],	[Lecture/Écriture], [Lecture/Écriture],
          [1], [1],	[Lecture uniquement], [Aucun accès],
      )
    ],
    [11:10], [*`domain`*], [
      - Ces bits spécifient le numéro de domaine (0-15) auquel appartient la section.
      - Le registre DACR (Domain Access Control Register) du processeur détermine les droits d'accès globaux pour chaque domaine.
    ],
    [4:3], [], [_Should-Be-Zero_],
    [2], [*`B`*], [Ce bit indique si les écritures dans la mémoire de la section sont bufferisées.],
    [1:0], [*`type`*], [ `0b10` pour une section]
  ),
  caption: [section layout]
)

===== Page table pointer
Une entrée de la table de niveau 1 peut également pointer vers une page table de niveau 2. Cela permet de mapper des petites pages (4 KB).

#figure(
  table(
    columns: (auto, 1fr),
    inset: 10pt,
    align: (center, left),
    table.header([*Offset*], [*Description*]),
    [31:10], [Adresse physique de la table de niveau 2 (alignée sur 1 KB).],
    [9:5], [Reserved (SBZ).],
    [4], [NS : Non-secure bit.],
    [3], [Must be 0 (SBZ).],
    [2], [Always 0 (SBZ).],
    [1], [Bit 1 : 1 (indique un type Page Table).],
    [0], [Bit 0 : 1 (indique un type Page Table).]
  ),
  caption: [pointeur de table de niveau 2]
)

==== Format des tables de niveau 2
Les entrées de la table de niveau 2 sont également sur 32 bits. Chaque entrée correspond à une page de 4 KB.

#figure(
  table(
    columns: (auto, 1fr),
    inset: 10pt,
    align: (center, left),
    table.header([*Offset*], [*Description*]),
      [31:12], [Adresse physique des 20 bits les plus significatifs (base de 4 KB alignée).],
      [11], [Global/Non-global bit.],
      [10], [Non-secure bit.],
      [9], [Shared (S).],
      [8], [Access Flag (AF).],
      [7], [Non-exécutable (NX).],
      [6:5], [TEX (Bits 2:1).],
      [4], [Cacheable (C).],
      [3], [Bufferable (B).],
      [2], [TEX (Bit 0).],
      [1:0], [Type (0b10 pour une petite page).],
  ),
  caption: [entrée d'une table de niveau 2]
)

=== Configuration d'une MMU identité:

- Création d'une table de niveau 1 en mémoire: 4096x4 = 16kb.

==== Création d'une table de niveau 1 en mémoire: 4096x4 = 16kb.
Chaque entrée correspond à une section de 1 MB.
==== Initialiser la table de niveau 1
- Pour un mapping identité chaque entré vas pointer vers elle même.
- Il existe également des flags: configuration des droits, du cache
==== Configurer les registres CP15 pour activer la MMU
- Configurer le *Translation table register (TTBR)* avec l'addresse physique de la table de niveau 1.
- Configurer le *Domain Access Control Register (DACR)* pour définir les droits d'accès au domaines.
- Activer la MMU dans le registre de controle

===  Registres impliqués
Différent registres sont necessaire pour mettre configurer et activer la mmu.

==== Translation Table Base Register 0 (TTBR0)
Le registre pointe vers la table de niveau 1 (alignée sur 16 KB): il contient l'addresse physique de la table.

#figure(
  table(
    columns: (auto, auto, 1fr),
    inset: 10pt,
    align: (center, center, left),
    table.header([*Offset*], [*Name*], [*Description*]),
    [31:7], [*TTB0*], [
      Translation table base address, bits `[31:x]`, where x is 14-(TTBCR.N). Register bits `[x-1:7]` are RES0, with the additional requirement that if these bits are not all zero, this is a misaligned translation table base address, with effects that are CONSTRAINED UNPREDICTABLE, and must be one of the following:
      - Register bits `[x-1:7]` are treated as if all the bits are zero. The value read back from these bits is either the value written or zero.
      - The result of the calculation of an address for a translation table walk using this register can be corrupted in those bits that are nonzero.
    ],
    [0,6], [*IRGN*], [
      Inner region bits. Bits `[0,6]` of this register together indicate the Inner Cacheability attributes for the memory associated with the translation table walks. The possible values of `IRGN[1:0]` are:

      #table(
        columns: (auto, auto),
        stroke: none,
        [0b00], [_Normal memory, Inner Non-cacheable._],
        [0b01], [_Normal memory, Inner Write-Back Write-Allocate Cacheable._],
        [0b10], [_Normal memory, Inner Write-Through Cacheable._],
        [0b11], [_Normal memory, Inner Write-Back no Write-Allocate Cacheable._],
      )

      The encoding of the IRGN bits is counter-intuitive, with register bit[6] being `IRGN[0]` and register bit[0] being `IRGN[1]`. This encoding is chosen to give a consistent encoding of memory region types and to ensure that software written for ARMv7 without the Multiprocessing Extensions can run unmodified on an implementation that includes the functionality introduced by the ARMv7 Multiprocessing Extensions.
    ],
    [5], [*NOS*], [],
    [4:3], [*RGN*], [],
    [2], [*IMP*], [ _implementation defined_ ],
    [1], [*S*], []
  ),
  caption: [TTBR0 register],
)

==== Domain Access Control Register (DACR)
Il contrôle les droits d'accès par domaine (16 domaines disponibles).
Par défaut :
- *Domaine 0    :* Client (les permissions dans les tables sont respectées).
- *Domaine 1-15 :* Pas d'accès ou manager (ignorer les permissions).

==== System Control Register (SCTLR)
 - Bit 0 (M) : Activer/désactiver la MMU.
 - Bit 2 (C) : Activer/désactiver le cache.
 - Bit 12 (I) : Activer/désactiver l'instruction cache.

#figure(
  table(
    columns: (auto, auto, 1fr),
    inset: 10pt,
    align: (center, center, left),
    table.header([*Offset*], [*Name*], [*Description*]),
    [31], [*DSSBS*], [],
    [30], [*TE*], [],
    [29], [*AFE*], [Access Flag Enable],
    [28], [*TRE*], [TEX remap enable],
    [27], [_Reserved_], [],
    [26], [_Reserved_], [],
    [25], [*EE*], [],
    [24], [_Reserved_], [],
    [23], [*SPAN*], [],
    [22], [_Reserved_], [],
    [21], [_Reserved_], [],
    [20], [*UWXN*], [],
    [19], [*WXN*], [
      Write permission implies XN (Execute-never). For the PL1&0 translation regime, this bit can force all memory regions that are writable to be treated as XN.

      The possible values of this bit are:
      / 0b0: This control has no effect on memory access permissions.
      / 0b1: Any region that is writable in the PL1&0 translation regime is forced to XN for accesses from software executing at PL1 or PL0.
    ],
    [18], [*nTWE*], [],
    [17], [_Reserved_], [],
    [16], [*nTWI*], [],
    [15], [_Reserved_], [],
    [14], [_Reserved_], [],
    [13], [*V*], [],
    [12], [*I*], [
      Instruction access Cacheability control, for accesses at EL1 and EL0:
      / 0b0: All instruction access to Normal memory from PL1 and PL0 are Non-cacheable for all levels of instruction and unified cache.\
        If the value of SCTLR.M is 0, instruction accesses from stage 1 of the PL1&0 translation regime are to Normal, Outer Shareable, Inner Non-cacheable, Outer Non-cacheable memory.
      / 0b1: All instruction access to Normal memory from PL1 and PL0 can be cached at all levels of instruction and unified cache.\
        If the value of SCTLR.M is 0, instruction accesses from stage 1 of the PL1&0 translation regime are to Normal, Outer Shareable, Inner Write-Through, Outer Write-Through memory.
    ],
    [11], [_Reserved_], [],
    [10], [*EnRCTX*], [],
    [9], [_Reserved_], [],
    [8], [*SED*], [],
    [7], [*ITD*], [],
    [6], [*UNK*], [],
    [5], [*CP15BEN*], [],
    [4], [*LSMAOE*], [],
    [3], [*nTLSMD*], [],
    [2], [*C*], [
      Cacheability control, for data accesses at EL1 and EL0:
      / 0b0: All data access to Normal memory from PL1 and PL0, and all accesses to the PL1&0 stage 1 translation tables, are Non-cacheable for all levels of data and unified cache.
      / 0b1: All data access to Normal memory from PL1 and PL0, and all accesses to the PL1&0 stage 1 translation tables, can be cached at all levels of data and unified cache.
    ],
    [1], [*A*], [
      Alignment check enable. This is the enable bit for Alignment fault checking at PL1 and PL0:
      / 0b0: Alignment fault checking disabled when executing at PL1 or PL0.
        Instructions that load or store one or more registers, other than load/store exclusive and load-acquire/store-release, do not check that the address being accessed is aligned to the size of the data element(s) being accessed.
      / 0b1: EL1 and EL0 stage 1 address translation enabled.
    ],
    [0], [*M*], [
      MMU enable for EL1 and EL0 stage 1 address translation.

      / 0b0: EL1 and EL0 stage 1 address translation disabled.
      / 0b1: EL1 and EL0 stage 1 address translation enabled.
    ],
  ),
  caption: [System Control Register (SCTLR)]
)
